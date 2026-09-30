#include "what_changed/util.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <poll.h>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

namespace wc {
namespace {
class UniqueFd {
public:
  explicit UniqueFd(int fd = -1) noexcept : fd_(fd) {}
  ~UniqueFd() { reset(); }
  UniqueFd(const UniqueFd&) = delete;
  UniqueFd& operator=(const UniqueFd&) = delete;

  int get() const noexcept { return fd_; }
  int release() noexcept { const int fd = fd_; fd_ = -1; return fd; }
  void reset(int replacement = -1) noexcept {
    if(fd_ >= 0) close(fd_);
    fd_ = replacement;
  }

private:
  int fd_;
};

class ChildOwner {
public:
  explicit ChildOwner(pid_t pid) noexcept : pid_(pid) {}
  ~ChildOwner() {
    if(pid_ <= 0) return;
    kill(pid_,SIGKILL);
    int status=0;
    while(waitpid(pid_,&status,0)<0&&errno==EINTR){}
  }
  ChildOwner(const ChildOwner&) = delete;
  ChildOwner& operator=(const ChildOwner&) = delete;
  void reaped() noexcept { pid_=-1; }
private:
  pid_t pid_;
};

pid_t wait_child(pid_t pid,int*status,int options){pid_t result;do{result=waitpid(pid,status,options);}while(result<0&&errno==EINTR);return result;}
}

std::string read_file_bounded(const std::filesystem::path& p,std::size_t max){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("cannot open file");std::string s;std::array<char,8192>b{};while(f){f.read(b.data(),static_cast<std::streamsize>(b.size()));auto n=f.gcount();if(n>0){if(s.size()+static_cast<std::size_t>(n)>max)throw std::runtime_error("file exceeds size limit");s.append(b.data(),static_cast<std::size_t>(n));}}if(f.bad())throw std::runtime_error("read failed");return s;}
void atomic_write_private(const std::filesystem::path& p,const std::string& data){if(p.empty())throw std::runtime_error("empty output path");struct stat st{};if(lstat(p.c_str(),&st)==0)throw std::runtime_error("refusing to overwrite existing path");if(errno!=ENOENT)throw std::runtime_error("cannot inspect output path");std::filesystem::path directory=p.parent_path();if(directory.empty())directory=std::filesystem::path(".");const std::filesystem::path filename_path=p.filename();const auto&filename=filename_path.native();std::string temp_filename;temp_filename.reserve(filename.size()+12);temp_filename.push_back('.');temp_filename.append(filename);temp_filename.append(".tmp.XXXXXX");std::filesystem::path temp_path=directory;temp_path/=temp_filename;const auto&native_temp_path=temp_path.native();std::vector<char>buf(native_temp_path.begin(),native_temp_path.end());buf.push_back('\0');UniqueFd fd(mkstemp(buf.data()));if(fd.get()<0)throw std::runtime_error("cannot create temporary output");try{if(fchmod(fd.get(),0600)!=0)throw std::runtime_error("cannot set output permissions");std::size_t off=0;while(off<data.size()){ssize_t n=write(fd.get(),data.data()+off,data.size()-off);if(n<0){if(errno==EINTR)continue;throw std::runtime_error("output write failed");}off+=static_cast<std::size_t>(n);}if(fsync(fd.get())!=0)throw std::runtime_error("output sync failed");const int output_fd=fd.release();if(close(output_fd)!=0)throw std::runtime_error("output close failed");long rc=syscall(SYS_renameat2,AT_FDCWD,buf.data(),AT_FDCWD,p.c_str(),1U);if(rc!=0){if(errno==ENOSYS||errno==EINVAL){if(link(buf.data(),p.c_str())!=0)throw std::runtime_error("atomic no-replace link failed");if(unlink(buf.data())!=0)throw std::runtime_error("temporary unlink failed");}else throw std::runtime_error("atomic no-replace rename failed");}}catch(...){unlink(buf.data());throw;}}

namespace {constexpr std::array<std::uint32_t,64>K={0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};inline std::uint32_t rr(std::uint32_t x,unsigned n){return(x>>n)|(x<<(32-n));}
class Sha{std::array<std::uint32_t,8>h{0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};std::array<unsigned char,64>b{};std::uint64_t bits=0;std::size_t used=0;void block(){std::array<std::uint32_t,64>w{};for(unsigned i=0;i<16;i++)w[i]=(std::uint32_t(b[i*4])<<24)|(std::uint32_t(b[i*4+1])<<16)|(std::uint32_t(b[i*4+2])<<8)|b[i*4+3];for(unsigned i=16;i<64;i++){auto s0=rr(w[i-15],7)^rr(w[i-15],18)^(w[i-15]>>3);auto s1=rr(w[i-2],17)^rr(w[i-2],19)^(w[i-2]>>10);w[i]=w[i-16]+s0+w[i-7]+s1;}auto a=h[0],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],hh=h[7],bb=h[1];for(unsigned i=0;i<64;i++){auto s1=rr(e,6)^rr(e,11)^rr(e,25),ch=(e&f)^((~e)&g),t1=hh+s1+ch+K[i]+w[i],s0=rr(a,2)^rr(a,13)^rr(a,22),maj=(a&bb)^(a&c)^(bb&c),t2=s0+maj;hh=g;g=f;f=e;e=d+t1;d=c;c=bb;bb=a;a=t1+t2;}h[0]+=a;h[1]+=bb;h[2]+=c;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=hh;used=0;}
public:void add(const unsigned char*p,std::size_t n){bits+=static_cast<std::uint64_t>(n)*8;while(n){auto take=std::min(n,b.size()-used);std::memcpy(b.data()+used,p,take);used+=take;p+=take;n-=take;if(used==64)block();}}std::string finish(){auto total=bits;b[used++]=0x80;if(used>56){while(used<64)b[used++]=0;block();}while(used<56)b[used++]=0;for(int i=7;i>=0;--i)b[used++]=static_cast<unsigned char>((total>>(i*8))&255);block();std::ostringstream o;o<<std::hex<<std::setfill('0');for(auto x:h)o<<std::setw(8)<<x;return o.str();}};}
std::string sha256_file_nofollow(const std::filesystem::path&p,std::uint64_t max,std::uint64_t&size){int fd=open(p.c_str(),O_RDONLY|O_CLOEXEC|O_NOFOLLOW);if(fd<0)throw std::runtime_error("unreadable");struct stat st{};if(fstat(fd,&st)!=0||!S_ISREG(st.st_mode)){close(fd);throw std::runtime_error("not regular file");}if(static_cast<std::uint64_t>(st.st_size)>max){close(fd);throw std::runtime_error("too large");}Sha h;std::array<unsigned char,16384>b{};size=0;for(;;){ssize_t n=read(fd,b.data(),b.size());if(n==0)break;if(n<0){if(errno==EINTR)continue;close(fd);throw std::runtime_error("read error");}size+=static_cast<std::uint64_t>(n);if(size>max){close(fd);throw std::runtime_error("too large");}h.add(b.data(),static_cast<std::size_t>(n));}close(fd);return h.finish();}
std::string utc_now(){auto t=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());std::tm tm{};gmtime_r(&t,&tm);std::ostringstream o;o<<std::put_time(&tm,"%Y-%m-%dT%H:%M:%SZ");return o.str();}
std::string sanitize_terminal(const std::string&s){std::string o;for(unsigned char c:s){if(c=='\n'||c=='\t'||(c>=0x20&&c!=0x7f))o+=static_cast<char>(c);else o+='?';}return o;}
std::string sanitize_repo_url(const std::string&s){auto v=trim(s);auto scheme=v.find("://");if(scheme!=std::string::npos){auto begin=scheme+3,at=v.find('@',begin),slash=v.find('/',begin);if(at!=std::string::npos&&(slash==std::string::npos||at<slash))v.erase(begin,at-begin+1);auto q=v.find_first_of("?#");if(q!=std::string::npos)v.erase(q);}return v;}
std::vector<std::string> split(const std::string&s,char d){std::vector<std::string>r;std::string x;std::istringstream i(s);while(std::getline(i,x,d))r.push_back(x);return r;}
std::string trim(std::string s){while(!s.empty()&&std::isspace(static_cast<unsigned char>(s.back())))s.pop_back();std::size_t i=0;while(i<s.size()&&std::isspace(static_cast<unsigned char>(s[i])))++i;return s.substr(i);}
std::optional<std::string> executable_path(const std::vector<std::string>&v){const auto found=std::find_if(v.begin(),v.end(),[](const std::string&p){return access(p.c_str(),X_OK)==0;});if(found==v.end())return std::nullopt;return *found;}
CommandResult run_command(const std::string&exe,const std::vector<std::string>&args,unsigned timeout,std::size_t max){int outp[2];if(pipe2(outp,O_CLOEXEC|O_NONBLOCK))throw std::runtime_error("pipe failed");UniqueFd out_read(outp[0]);UniqueFd out_write(outp[1]);int errp[2];if(pipe2(errp,O_CLOEXEC|O_NONBLOCK))throw std::runtime_error("pipe failed");UniqueFd err_read(errp[0]);UniqueFd err_write(errp[1]);pid_t pid=fork();if(pid<0)throw std::runtime_error("fork failed");if(pid==0){dup2(out_write.get(),STDOUT_FILENO);dup2(err_write.get(),STDERR_FILENO);close(out_read.get());close(out_write.get());close(err_read.get());close(err_write.get());std::vector<char*>av;av.reserve(args.size()+2);av.push_back(const_cast<char*>(exe.c_str()));std::transform(args.begin(),args.end(),std::back_inserter(av),[](const std::string&a){return const_cast<char*>(a.c_str());});av.push_back(nullptr);execv(exe.c_str(),av.data());_exit(127);}ChildOwner child(pid);out_write.reset();err_write.reset();CommandResult r;auto start=std::chrono::steady_clock::now();bool od=false,ed=false;int status=0;for(;;){pollfd fds[2]={{out_read.get(),POLLIN,0},{err_read.get(),POLLIN,0}};poll(fds,2,20);char b[4096];auto drain=[&](int descriptor,std::string&dst,bool&done){for(;;){ssize_t n=read(descriptor,b,sizeof b);if(n>0){auto count=static_cast<std::size_t>(n);if(dst.size()+count>max){auto room=max>dst.size()?max-dst.size():0;dst.append(b,room);r.timed_out=true;kill(pid,SIGKILL);break;}dst.append(b,count);}else{if(n==0)done=true;break;}}};drain(out_read.get(),r.out,od);drain(err_read.get(),r.err,ed);if(r.timed_out){wait_child(pid,&status,0);child.reaped();break;}pid_t w=wait_child(pid,&status,WNOHANG);if(w==pid){child.reaped();while(!od)drain(out_read.get(),r.out,od);while(!ed)drain(err_read.get(),r.err,ed);break;}if(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count()>timeout){r.timed_out=true;kill(pid,SIGKILL);wait_child(pid,&status,0);child.reaped();break;}}if(r.timed_out)r.exit_code=124;else if(WIFEXITED(status))r.exit_code=WEXITSTATUS(status);else r.exit_code=128+(WIFSIGNALED(status)?WTERMSIG(status):0);return r;}
}
