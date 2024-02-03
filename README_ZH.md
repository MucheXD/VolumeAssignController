# VolumeAssignController

这是一个简单的程序，帮助你一键为磁盘卷分配卷标与取消分配。这个程序基于 `diskpart.exe`

**这个项目不是MucheXD的计划内项目，因此可能存在许多潜在的问题，~~与不堪入目的码风~~**

## 使用说明

### 配置

程序配置保存在 `config.mtd` 中（你可以在程序运行目录下找到它）。使用任意文本编辑器（建议使用 JSON 支持的编辑器）即可打开它。程序配置遵循 JSON 格式，具体编写参见下例：

```json
{
    "configs": [
        {
            "name": "default",
            "disk": 0,
            "volume": 0,
            "char": "Z"
        }
    ],
    "default": 0
}
```

`configs` 是一个数据组，包含所有的子配置以供稍后在程序运行时快速切换。所有的子配置对象包含 `name` `disk` `volume` `char` 四个键值对，下面列出它们的作用：

| key    | value  | description                                        |
| ------ | ------ | -------------------------------------------------- |
| name   | string | 显示名称                                               |
| disk   | int    | 磁盘索引 [关于索引](#%E5%85%B3%E4%BA%8E%E7%B4%A2%E5%BC%95) |
| volume | int    | 卷索引 [关于索引](#%E5%85%B3%E4%BA%8E%E7%B4%A2%E5%BC%95)  |
| char   | string | 目标分配盘符。如果此项缺省，则会使用最小的可用字母。                         |

`default` 规定默认使用的子配置。

`disableWarning` 禁用磁盘保护警告。如果你尝试操作 C 盘，则会触发警告。不建议关闭该警告（除非你的系统盘盘符不为 C）。

### 分配 & 取消分配

简单的点击 `Assign` 或 `Unassign` 按钮即可！现行磁盘状态将显示在其上。

### 选择子配置

在下拉框中选择配置好的子配置，相关信息将显示在下方。

## 其它

### 声明

操作磁盘卷有时会导致一些问题，尤其是当有程序正在使用这些卷时。请务必自己小心使用！永远不要删除系统盘的盘符！**MucheXD不对任何故障或文件丢失负责。**

### 关于索引

请自行获取目标的索引！下面是基本方法:

1. 运行 `cmd.exe` （管理员模式下）

2. 运行命令 `diskpart`

3. 运行命令 `list disk` 并找到目标磁盘的索引

4. 运行命令 `select disk [目标磁盘索引]` 定位目标磁盘

5. 运行命令 `list volume` 获取目标卷的索引

### 建议的运行环境

- Windows 10 (x64) / Windows 11 (x64)

- Microsoft Visual C++ 2015-2022 Redistributable x64

### Copyright and License

This program uses the following compilers or support libraries

- Visual Studio 2022

- QT 6.5

This program is under the LGPL license.

For detailed license content, please refer to its corresponding official website.
