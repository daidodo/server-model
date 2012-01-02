#!/bin/bash

MYSQL_EXE="mysql -u root -p -e"

SQL="insert into mysql.user (Host, User, Password, Show_db_priv) values ('localhost', 'test', PASSWORD('test@pass'), 'Y') on duplicate key update Host='localhost', Password=PASSWORD('test@pass'), Show_db_priv='Y' ;
  flush privileges ;
  create database if not exists test ;
  grant all on test.* to 'test'@'localhost' ;"

$MYSQL_EXE "$SQL"
