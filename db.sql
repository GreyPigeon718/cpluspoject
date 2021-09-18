create database if not exists vod_system;

    use vod_system;
  
   create table if not exists tb_video(
       id int primary key auto_increment,
        name varchar(32),
       vdesc text,
       video_url varchar(255),
       image_url varchar(255),
      ctime datetime
  );
  
  insert tb_video values(null, "飞机大战", "这是一部非常精彩的家庭喜剧", "/video/plane.mp4    ", "/image/plane.jpg",now()); 
