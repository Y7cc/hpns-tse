
Distribution Training Simulator in NS-3
================================

## 配置安装

初始化`Git`子模块:

```bash
git submodule update --init --recursive
```

### 安装第三方库

[安装sdplog与protobuf](./src/hpns-dts/extern/INSTALL.md)

### 配置与编译ns-3

配置：

```bash
./waf configure --enable-mpi --enable-tests
```

Debug模式配置：

```bash
./waf configure --build-profile=debug --enable-examples
```

编译：

```bash
./waf
```

运行：

```bash
cd scratch/dts
bash ./run.sh
```

## Git使用

按照“配置安装”中的步骤初始化代码，写代码前在master分支上拉取最新代码：


```bash
git checkout master
git pull origin master
```

创建一个新分支：

```bash
git checkout -b new_branch
```

开发后将代码提交到远程新分支：

```bash
git add .
git commit -m "commit message"
git push origin new_branch
```

在GitHub上创建一个Pull Request，等待代码审查。