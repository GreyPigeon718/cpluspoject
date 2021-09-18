#include <fstream>
#include <boost/algorithm/string.hpp>
#include "db.hpp"
#include "httplib.h"
//#define WWWROOT "./wwwroot"
const std::string WWWROOT = "./wwwroot";
// /video/**.mp4
using namespace std;
using namespace httplib;
using namespace vod_system;
vod_system::TableVod *tb_video;

void VideoDelete(const Request& req, Response& rsp) 
{

  //req.path = /video/?
  //1.获取视频id
  int video_id = std::stoi(req.matches[1]);
  //2.从数据库中获取到对应视频信息
  Json::Value json_rsp;
  Json::FastWriter writer;
  Json::Value video; 
  bool ret = tb_video->GetOne(video_id, &video);
  if (ret == false) {
    std::cout << "mysql get video info failed!" << std::endl;
    rsp.status = 500;
    json_rsp["rsult"] = false;
    json_rsp["reason"] = "mysql get video info failed!";
    rsp.body = writer.write(json_rsp);
    rsp.set_header("Content-Type", "application/json");
    return ;
  }
  //3.删除视频文件,封面图片文件
  std::string vpath = WWWROOT + video["video_url"].asString();
  std::string ipath = WWWROOT + video["image_url"].asString();
  unlink(vpath.c_str());
  unlink(ipath.c_str());
  //4.删除数据库中的数据
  ret = tb_video->Delete(video_id);
  if (ret == false) {
    rsp.status = 500;
    std::cout << "mysql delete video failed!\n";
    return;
  }
  return;
}

void VideoUpdate(const Request& req, Response& rsp) 
{
  int video_id = std::stoi(req.matches[1]);
  Json::Value video;
  Json::Reader reader;
  bool ret = reader.parse(req.body, video);
  if (ret == false) {
    std::cout << "update video:parse video json failed!\n";
    rsp.status = 400;
    return;
  }
  ret = tb_video->Update(video_id, video);
  if (ret == false) {
    std::cout << "update video:mysql update failed!\n";
    rsp.status = 500;
    return;
  }
  return ;
}

void VideoGetAll(const Request& req, Response& rsp) 
{
  Json::Value videos;
  Json::FastWriter writer;
  bool ret = tb_video->GetAll(&videos);
  if (ret == false) {
    std::cout << "getall videos:mysql operation failed!\n";
    rsp.status = 500;
    return ;
  }
  rsp.body = writer.write(videos);
  rsp.set_header("Content-Type", "application/json");
  return ;
}

void VideoGetOne(const Request& req, Response& rsp) 
{
  int video_id = std::stoi(req.matches[1]);
  Json::Value video;
  Json::FastWriter writer;
  bool ret = tb_video->GetOne(video_id, &video);
  if (ret == false) {
    std::cout << "getone videos:mysql operation failed!\n";
    rsp.status = 500;
    return ;
  }
  rsp.body = writer.write(video);
  rsp.set_header("Content-Type", "application/json");
  return ;
}

#define VIDEO_PATH "/video/"
#define IMAGE_PATH "/image/"
void VideoUpload(const Request& req, Response& rsp) 
{
  auto ret = req.has_file("video_name");
  if (ret == false) {
    std::cout << "have no video_name!\n";
      rsp.status = 400;
    return;

  }
  const auto& file = req.get_file_value("video_name");
  const std::string& vname = file.content;

  ret = req.has_file("video_desc");
  if (ret == false) {
    std::cout << "have no video_desc!\n";
    return;

  }
  const auto& file1 = req.get_file_value("video_desc");
  const std::string& vdesc = file1.content;

  ret = req.has_file("video_file");
  if (ret == false) {
    std::cout << "have no video_file!\n";
    rsp.status = 400;
    return;

  }
  const auto& file2 = req.get_file_value("video_file");
  const std::string& vfile = file2.filename;
  const std::string& vcont = file2.content;

  ret = req.has_file("image_file");
  if (ret == false) {
    std::cout << "have no image_file!\n";
    rsp.status = 400;
    return;

  }
  const auto& file3 = req.get_file_value("image_file");
  const std::string& ifile = file3.filename;
  const std::string& icont = file3.content;
  
  std::string vurl = VIDEO_PATH + file2.filename;
  vod_system::Util::WriteFile(WWWROOT + vurl, file2.content);
  std::string iurl = IMAGE_PATH + file3.filename;
  vod_system::Util::WriteFile(WWWROOT + iurl, file3.content);
  
  Json::Value video;
  video["name"] = vname;
  video["vdesc"] = vdesc;
  video["video_url"] = vurl;
  video["image_url"] = iurl;
  ret = tb_video->Insert(video);
  if (ret == false) {
    rsp.status = 500;
    std::cout << "insert video:mysql operation failed" << std::endl;
    return ;
  }
  return ;
}

bool ReadFile(const string & name, string * body){
    ifstream ifile;
    ifile.open(name, ios::binary);
    if(!ifile.is_open()){
        printf("open file failed:%s\n",name.c_str());
        ifile.close();
        return false;
    }
    ifile.seekg(0, ios::end);
    uint64_t length = ifile.tellg();
    ifile.seekg(0, ios::beg);
    body->resize(length);
    ifile.read(&(*body)[0],length);
    if(ifile.good() == false){
        printf("read file failed:%s\n",name.c_str());
        ifile.close();
        return false;
    }
    ifile.close();
    return true;
}
void VideoPlay(const Request &req, Response &rsp)
{
    Json::Value video;
    int video_id = stoi(req.matches[1]);
    bool ret = tb_video->GetOne(video_id,&video);
    if(ret == false){
        cout << "getone video : mysql operation failed!\n";
        rsp.status = 500;
        return;
    }
    string newstr = video["video_url"].asString();
    string oldstr ="{{video_url}}";
    string play_html ="./wwwroot/single-video.html";
    ReadFile(play_html,&rsp.body);
    boost::algorithm::replace_all(rsp.body,oldstr,newstr);
    rsp.set_header("Content-Type","text/html");
    return ;
}
int main() 
{
  tb_video = new vod_system::TableVod();
  Server srv;
  //正则表达式 \d-匹配一个数字字符 +匹配字符一次或多次
  //R"(string)"去除括号中字符串中每个字符的特殊含义
  srv.set_base_dir("./wwwroot");
  srv.Delete(R"(/video/(\d+))", VideoDelete);
  srv.Put(R"(/video/(\d+))", VideoUpdate);
  srv.Get(R"(/video)", VideoGetAll);
  srv.Get(R"(/video/(\d+))", VideoGetOne);
  srv.Post(R"(/video)", VideoUpload);
  srv.Get(R"(/play/(\d+))",VideoPlay);
  srv.listen("0.0.0.0", 9000);
  return 0;
}

void test() {
  vod_system::TableVod tb_vod;
  Json::Value val;
  Json::StyledWriter write;
  //tb_vod.GetAll(&val);
  tb_vod.GetOne(1, &val);
  std::cout << write.write(val) << std::endl;
#if 0
  val["name"] = "蜜雪冰城";
  val["vdesc"] = "恐怖,电影";
  val["video_url"] = "/video/saw.mp4";
  val["image_url"] = "/video/saw.jpg";
  //tb_vod.Insert(val);
  //tb_vod.Update(2, val);
  tb_vod.Delete(2);
#endif
  return;
}

