# LinHub

面向 **银河麒麟 / 优麒麟** 的专业终端连接管理器。目标是在 Linux 上提供与 MobaXterm 相当的多协议工作台，同时吸收 Xshell 的会话组织、日志和发送栏，并用 SQLite 保存**任意数量**的会话与连接历史。

## 为什么做这个

| 痛点 | LinHub 的做法 |
| --- | --- |
| MobaXterm 免费版会话数量受限，且为 Windows 软件 | 麒麟原生 Qt 应用，会话库无数量上限 |
| Xshell 会话管理强，但图形/SFTP/X11 弱于 Moba | 左侧会话树 + 标签终端 + 右侧 SFTP + 可选 X11 转发 |
| 连接记录不好查 | 每次连接写入 `connection_history`，可按会话回溯 |

## 首版功能

- **协议**：SSH、SFTP、Telnet、本地 Shell、串口（minicom）
- **会话库**：分组、收藏、标签、备注、搜索、复制会话、导入/导出、从 `~/.ssh/config` 导入
- **终端**：自研 VT/ANSI 仿真、256 色、真彩色 SGR、中文宽字符、输入法、右键复制/粘贴/清屏
- **Xshell 风格**：会话日志、底部发送栏、快速命令、广播到全部标签
- **MobaXterm 风格**：快速连接条、标签页、X11 转发、侧栏 SFTP、SSH 端口转发
- **SFTP**：拖拽上传下载、文件夹上传、新建/重命名、传输历史
- **历史**：连接开始/结束时间、时长、结果、日志路径

## 在麒麟系统上编译

依赖（以银河麒麟 V10 / Ubuntu Kylin 为例）：

```bash
sudo apt update
sudo apt install -y build-essential cmake \
    qtbase5-dev qtbase5-dev-tools libqt5sql5-sqlite \
    openssh-client telnet minicom
```

若系统是 Qt 6：

```bash
sudo apt install -y qt6-base-dev qt6-base-dev-tools
```

编译：

```bash
cd LinHub
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

运行：

```bash
./build/linhub
```

安装：

```bash
sudo cmake --install build
```

菜单中会出现 “LinHub 终端连接管理器”。

## 数据位置

会话库与日志默认写在用户目录：

```
~/.local/share/LinHub/linhub.sqlite
~/.local/share/LinHub/logs/
```

密码按本机标识做本地混淆存储，**生产环境请优先使用 SSH 私钥 / ssh-agent**。

## 架构

```
会话树 / 历史          标签终端 (PTY + VT)         SFTP
SQLite 会话库    <--->  OpenSSH / telnet / bash  <--> scp/ssh ls
```

系统 OpenSSH 负责认证与算法，LinHub 负责会话资产、终端体验和记录。

## 后续规划

- 内置 libssh SFTP（不依赖外部 ssh 列目录）
- RDP / VNC 标签（FreeRDP / TigerVNC）
- 端口转发与隧道面板
- 会话加密备份与多机同步
- 宏脚本与按键映射
