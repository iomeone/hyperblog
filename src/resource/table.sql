CREATE DATABASE IF NOT EXISTS blog character=utf8; USE blog;
CREATE TABLE IF NOT EXISTS blog_table (
 blog_id INT PRIMARY KEY NOT NULL AUTO_INCREMENT,
 title VARCHAR(128),
 content TEXT,
 tag_id INT,
 create_time VARCHAR(50)
);
CREATE TABLE IF NOT EXISTS tag_table (
tag_id INT NOT NULL PRIMARY KEY AUTO_INCREMENT,
tag_name VARCHAR(50)
);