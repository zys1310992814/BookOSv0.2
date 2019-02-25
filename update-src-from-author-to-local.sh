#!/bin/bash
#这个bash的作用是为了保持本地代码与原作者的代码保持同步
set -e
#判断是否增加了原作者的远程源
  if [ ! `grep -c "upstream" ./.git/config` -eq '0' ]; then
    echo "原作者的远程源已经存在!" 
  else 
    git remote add upstream git@github.com:huzichengdevelop/BookOSv0.2.git
    echo "Done，原作者的远程源已经添加!"
    git remote -v
  fi
  echo "与主repo合并..."
#获取原作者的更新
  git fetch upstream
#原作者的master分支与本地master分支进行合并
  git merge upstream/master
#假如你想丢弃你在本地的所有改动与提交，获取原作者最新的版本，并将你本地主分支指向它：
#如何有自己的代码，不要运行下面的命令
#  git fetch upstream
#  git reset --hard upstream/master
