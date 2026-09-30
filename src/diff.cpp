#include "what_changed/diff.hpp"
#include <algorithm>
#include <cctype>
#include <set>
#include <sstream>

namespace wc {
namespace {std::string value(const Json&j,const std::string&k){if(!j.is_object()||!j.contains(k)||!j.at(k).is_string())return{};return j.at(k).as_string();}int version_cmp(const std::string&a,const std::string&b){auto pa=a,pb=b;const auto punctuation=[](char c){return !std::isalnum(static_cast<unsigned char>(c));};std::replace_if(pa.begin(),pa.end(),punctuation,' ');std::replace_if(pb.begin(),pb.end(),punctuation,' ');std::istringstream ia(pa),ib(pb);std::string x,y;while(ia>>x){if(!(ib>>y))return 1;bool nx=!x.empty()&&std::all_of(x.begin(),x.end(),[](char c){return std::isdigit(static_cast<unsigned char>(c));});bool ny=!y.empty()&&std::all_of(y.begin(),y.end(),[](char c){return std::isdigit(static_cast<unsigned char>(c));});if(nx&&ny){auto sx=x.find_first_not_of('0'),sy=y.find_first_not_of('0');auto xx=sx==std::string::npos?"0":x.substr(sx),yy=sy==std::string::npos?"0":y.substr(sy);if(xx.size()!=yy.size())return xx.size()<yy.size()?-1:1;if(xx!=yy)return xx<yy?-1:1;}else if(x!=y)return x<y?-1:1;}if(ib>>y)return-1;return 0;}}
Diff compare(const Snapshot&a,const Snapshot&b,bool transient){
  Diff d; std::set<std::string>cats; for(const auto&x:a.categories)cats.insert(x.first); for(const auto&x:b.categories)cats.insert(x.first);
  for(const auto&cat:cats){
    auto ai=a.categories.find(cat),bi=b.categories.find(cat);
    if(ai==a.categories.end()||bi==b.categories.end()||ai->second.status=="unavailable"||bi->second.status=="unavailable"){d.unavailable.push_back(cat+": comparison unavailable because one snapshot did not collect this category");continue;}
    std::set<std::string>keys;for(const auto&e:ai->second.entries)keys.insert(e.first);for(const auto&e:bi->second.entries)keys.insert(e.first);
    for(const auto&k:keys){
      auto x=ai->second.entries.find(k),y=bi->second.entries.find(k);
      if(x==ai->second.entries.end()){auto type=cat=="packages"?"installed":"added";d.changes.push_back({cat,k,type,"Observed a new "+cat+" entry.",Json(),y->second,cat=="services"&&value(y->second,"state_kind")=="transient"});continue;}
      if(y==bi->second.entries.end()){d.changes.push_back({cat,k,"removed","Observed that a previously collected "+cat+" entry is no longer present.",x->second,Json(),cat=="services"&&value(x->second,"state_kind")=="transient"});continue;}
      if(x->second.dump()==y->second.dump())continue;
      std::string type="modified",why="Observed fields changed.";
      if(cat=="packages"){auto va=value(x->second,"version"),vb=value(y->second,"version"),aa=value(x->second,"architecture"),ab=value(y->second,"architecture");if(aa!=ab){type="architecture-changed";why="Observed package architecture changed.";}else{int c=version_cmp(va,vb);type=c<0?"upgraded":c>0?"downgraded":"version-changed";why="Observed package version changed.";}}
      else if(cat=="services"){auto ea=value(x->second,"enabled"),eb=value(y->second,"enabled");if(ea!=eb&&(eb=="enabled"||eb=="disabled")){type=eb;why="Observed persistent service enablement changed.";}}
      bool ctx=false;
      if(cat=="services"){
        auto xo=x->second.as_object(),yo=y->second.as_object();
        xo.erase("active");xo.erase("sub");xo.erase("state_kind");yo.erase("active");yo.erase("sub");yo.erase("state_kind");
        ctx=Json(std::move(xo)).dump()==Json(std::move(yo)).dump();
      }
      if(!ctx||transient)d.changes.push_back({cat,k,type,why,x->second,y->second,ctx});
    }
  }
  return d;
}
}
