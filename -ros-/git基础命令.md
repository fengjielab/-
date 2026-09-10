# Git 基础命令

## 一、今天掌握的知识点

### 1. 第一次把远程项目下载到 Nano

~~~bash
cd ~/目标目录
git clone https://github.com/用户名/仓库名.git
cd 仓库名
~~~

git clone 会把远程仓库复制到 Nano，并自动建立本地仓库与远程仓库的关联。

### 2. 拉取远程仓库的最新代码

项目已经克隆过时，进入项目目录后执行：

~~~bash
cd ~/项目目录
git pull origin main
~~~

如果远程主分支叫 master，则执行：

~~~bash
git pull origin master
~~~

看到 Fast-forward，通常表示远程代码已经成功更新到本地。

### 3. 查看当前仓库状态

~~~bash
git status
~~~

常见输出：

~~~text
On branch main
~~~

表示当前位于本地 main 分支。

~~~text
Your branch is up to date with 'origin/main'.
~~~

表示本地 main 分支和远程 origin/main 已同步。

~~~text
nothing to commit, working tree clean
~~~

表示当前没有未提交的文件修改，工作区是干净的。这不代表没有代码，只代表没有新的改动。

### 4. 查看提交历史

~~~bash
git log --oneline --all -5
~~~

- --oneline：每条提交只显示一行
- --all：显示所有分支的提交
- -5：只显示最近 5 条提交

例如：

~~~text
878f634 (HEAD -> main, origin/main, origin/HEAD) 测试
~~~

- 878f634：提交编号的简写
- HEAD -> main：当前所在的本地分支
- origin/main：远程仓库的 main 分支
- origin/HEAD：远程仓库默认分支的指针

这些指针指向同一个提交时，说明本地和远程当前同步。

### 5. 终端颜色不是报错

Git 会用颜色区分提交编号和分支指针。origin/main 或 origin/HEAD 显示为红色时，只是终端的颜色配置，不代表错误。目录通常显示为蓝色，也只是表示它是文件夹。

## 二、修改代码后提交并上传到 GitHub

~~~bash
git status
git add 文件名
git commit -m 说明本次修改
git push origin main
~~~

例如：

~~~bash
git add keyboard_control.py
git commit -m 修改键盘控制程序
git push origin main
~~~

添加当前目录下所有修改：

~~~bash
git add .
~~~

注意：

- git commit 只是在 Nano 本地保存提交
- git push 才会把提交上传到 GitHub
- 上传后可以在 GitHub 网页刷新仓库查看

## 三、常用基础命令

查看远程仓库地址：

~~~bash
git remote -v
~~~

查看当前分支：

~~~bash
git branch
~~~

切换分支：

~~~bash
git switch 分支名
~~~

创建并切换到新分支：

~~~bash
git switch -c 新分支名
~~~

查看最近一次提交修改了哪些文件：

~~~bash
git show --stat
~~~

查看最近一次提交的详细内容：

~~~bash
git show
~~~

查看当前目录文件（Linux 终端命令，不是 Git 命令）：

~~~bash
ls
ls -la
~~~

## 四、日常操作流程

### 在 Nano 上获取 GitHub 最新代码

~~~bash
cd ~/项目目录
git status
git pull origin main
~~~

### 在 Nano 上修改并上传代码

~~~bash
git status
git add .
git commit -m 说明修改内容
git push origin main
~~~

### 确认是否同步

~~~bash
git status
git log --oneline --all -5
~~~

如果看到 up to date 和 working tree clean，通常表示本地、远程和工作区都已同步。

## 五、常见问题

### 为什么 git status 看不到代码？

git status 只显示有没有未提交的修改，不显示文件列表或提交历史。查看文件使用：

~~~bash
ls
git log --oneline --all -5
~~~

### 为什么提交后 GitHub 上看不到变化？

可能只执行了 git commit，还没有执行：

~~~bash
git push origin main
~~~

### 当前目录不是 Git 仓库怎么办？

如果提示 not a git repository，先进入正确的项目目录：

~~~bash
cd ~/项目目录
git status
~~~

### GitHub 要求输入密码怎么办？

使用 HTTPS 时，GitHub 通常需要 Personal Access Token，而不是 GitHub 登录密码。也可以配置 SSH 密钥，减少重复认证。

