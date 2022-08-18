<h1 style="text-align: center;"> 本科生实验报告 </h1>
<br />
实验课程： 操作系统
实验名称： 期末大作业：文件系统
<br />
专业名称： 网络空间安全
<br />
学生学号： 20337021
<br />
实验地点： 我的家
<br />
实验成绩：
<br />
报告时间：
<br />
<br />

## 实验要求：
自己实现一个操作系统，并支持FAT 12 文件系统， 实现函数open, read, write, close, 并支持命令 cd, pwd, ls, cat, echo。
## FAT 12 文件系统架构
FAT 12 文件系统时早期被Microsoft 发明的。在我们实验中使用一个虚拟的软盘（floppy disk) 来作为secondary storage。
FAT12 的架构如下：
<img src="fatstructure.png">
这个架构就是备用储存里面的储存架构，当中最低地址放着操作系统的应道主引导程序，之后时两个File allocation table（FAT）, 有两个的原因是因为当一个FAT损坏时，另外一个还可以当中备用。它的功能就是给出文件在数据区的位置。之后是根目录去和数据区。根目录存放着每一个文件或者文件夹的目录项，目录项存放着文件的元信息，包括文件名， 文件大小，文件位置。数据区就是放着所有文件里面的数据，通过目录项FAT表，就可以找到所需要文件的数据，并进行读写。

<br /> 
在MBR 里面放着文件系统组成信息，例如每簇扇区数 (bytes per sector) = 512 和 每簇扇区数（sectors per cluster) = 1。

```
Boot jump instruction
Oem identifier                       MSWIN4.1 
Bytes per sector                     512
Sectors per cluster                  1
Reserved sectors                     1
Fat count                            2
Dir entry count                      224
Total sectors                        2880
Media descriptor type                0F0h ;F0 = 3.5" floppydisk
Sectors per fat                      9    ; 9 sectors / fat
Sectors per track                    18
Heads                                2
Hidden sectors                       0
Large sector count                   0
```

这里给出了MBR区占用了一个 1个扇区，么个FAT表占用9个扇区，因此FAT 区总共占用了2*9 = 18个三区，之后根目录所占用的扇区数 
```
（Dir entry count * 32 + Bytes per sector - 1) / Bytes per sector = 14 个扇区
```
因此数据区占用了的扇区 = 总扇区 - 1 - 14 - 18 = 2880 - 33 = 2847，这也是簇的数量。

<br />
注意到FAT12当中的12是我们最小所需要给出每个扇区一个地址的比特数,因为我们有 2880个扇区，且 

2^11 < 2880 < 2^12



![alt text]./assets/sectorSizes.png) 
 如果一个文件大过一个簇的大小（512 bytes）， 那么就需要通过FAT 表找出它在512 bytes 以上的数据缩放在的簇的位置。注意FAT的文件系统都是链式分配的。我们现在讨论如何从一个在根目录里的目录项以及FAT表来获取文件数据。首先我们已知的一个文件名 name, 它有 8.3 格式（名，扩充）格式，需要转换成FAT长度位11的格式， 例如 "test.txt" => fat_name = "TEST    TXT"(如果是文件架的话，FAT格式最后三个格会为空）。通过逐一读取在根目录的目录项，直到找到一个目录项direntry, 符合direntry.name == fat_name。找出对应的目录项后，通过开始簇direntry.firstcluster 计算出对应FAT。注意到fat12 是使用12位来作为数据的地址。
 
 ```
 
uint32_t FAT_NextCluster(uint32_t currentCluster)
{

    uint32_t fatIndex = currentCluster * 3 / 2;
    if(currentCluster % 2 == 0)
    {
		return (*(uint16_t far*)(g_Fat + fatIndex))& 0x0FFF;
    }
    else
    {
		return (*(uint16_t far*)(g_Fat + fatIndex)) >> 4; 
    }
}
 ```
 把FAT表看为一个数组FAT, 
 如果目前的簇位偶数，FAT表项 = FAT[currentCluster * 3/2] 的高四位
 ；如是奇数，FAT表项 = FAT[currentCluster * 3/2] 的低四位。之前说过FAT12 是链式分配的，这是因为通过FAT表找出对应目前文件簇的项，该项的值就是对应目前文件簇的下一个簇的值。如果文件在目前的簇已经结束了，那么类似于链表使用NULL代表链表终止，如果对应的项为 0xFF8 - 0xFFF的值，就代表该簇为文件的最后一个簇。通过链式分配，解决到连续分配的问题。采用链接分配，每个文件是磁盘块（在这里叫做簇）的链表，磁盘块可能会散步在磁盘的任何地方。这解决了连续分配存在外来碎片的问题（external fragmentation), 空闲空间列表的任何块可以给用于满足请求。当创建文件时，并不需要说明文件的大小。只要有可用的空闲块，文件就可以继续郑家。因此，无需合并磁盘空间。
 
找到对应文件的簇以后，就需要计算每一个簇的地址。注意我们说过文件的数据是在数据区里面，因此簇的地址都是数据区的地址，也就是簇所在扇区，在这里叫做*物理扇区*，公式如下：
```
first_sector_of_cluster = (cluster - 2)*fat_boot->sectors_per_cluster =  + first_data_sector; 
```
注意到因为fat_boot->sectors_per_cluster = 1 (对于FAT12而言)，所以这个公式就是计算簇对应的扇区。
first_data_sector 就是对应于数据区的第一个扇区。
而first_data_sector = 1 + 18+ 14 = 33
因此
first_sector_of_cluster = 33 + cluster - 2

我门以下例子来讲解以下FAT 12 的原理：
<br />
![alt text]./assets/fat12Example.png)

从图中，File1.txt的逻辑扇区为2， 4，6 和 7。该文件的目录项的开始扇区FirstSector 为2， 也就是第一个数据扇区。在FAT表中，FAT表对应第2项的值为4，代表File1.txt的下一个数据扇区在逻辑扇区4。最后一个扇区为4，这是显然的因为图上FAT表第7项写着EOC。对于文件夹MyDir 也是类似的。
注意到在文件的目录项里的Attributes 就是用来分别出它是文件还是根文件里。
## 代码分析
我使用的是open watcom 的C编译器, nasm x86汇编器, 以及qemu模拟处理器，实现操作系统，并以文件系统为主要开发点实现文件系统 open, read, write, close 主要函数，以及相关cd, ls, cat, echo的命令。

在stage1文件夹里面定义了我们的引导程序boot.asm，它负责加载内核stage2.bin来执行。当中stage2.asm在根文件架里。在boot.asm里的load_kernel_loop 模块负责加载内核。
![alt text]./assets/load_kernel_loop.png)

在stage2文件夹里定义了stage2.bin的代码，其中要执行的是main.c的main函数里面的代码。
其中所依赖的程序代码在：
<br />

x86.asm： 通过x86语言定义的读取/写入扇区，重置disk的函数
<br />

disk.c：封装x86.asm的代码为c代码
<br />

fat.c: 实现文件系统的函数open, read, write, close

shellCmd.c: 实现命令函指令cat, echo, cd, ls

### <span> 1. </span> x86.asm
x86_Disk_Reset: 通过中断指令 int 13h, 中断号 AH = 00h 重置disk
<br />
![alt text]./assets/x86reset.png)

<br />
x86_Disk_Read: 通过中断指令 int 13h, 中断号 AH = 02h 读取扇区

![alt text]./assets/x86write.png)
<br />
x86_Disk_Write：
通过中断指令 int 13h, 中断号 AH = 03h 写人扇区 
![alt text]./assets/x86read.png)
<br />
x86_Disk_GetDriveParams:
通过中断指令 int 13h, 中断号 AH = 08h, 传递给参数 柱面，磁头，扇区的总数量。 
![alt text]./assets/x86params.png)
<br />
### <span> 2. </span> disk.c
我们有disk类，用来储存软盘 柱面，磁头，扇区的总数量。
DISK_Initialize （初始化disk类）：
![alt text]./assets/diskinitialize.png)
Disk_WriteSectors: 
![alt text]./assets/disk_write.png)
DISK_ReadSectors：读取扇区
![alt text]./assets/disk_read.png)
DISK_LBA2CHS（从lba地址转换到chs)
![alt text]./assets/lba2chs.png)
### <span> 3. </span> fat.c
目录项结构 (directory entry)
![alt text]./assets/directoryentry.png)
FAT_File:  (对文件的信息)
* Handler：文件处理
* isDirectory: 是否文件架 

* Position：目前在文件里的byte位置

* Size: 文件的byte大小

attributes: 文件的性质，例如只有读权限 (FAT_ATTRIBUTE_READ_ONLY), 是否文件夹(FAT_ATTRIBUTE_DIRECTORY) 
![alt text]./assets/fatfileandattributes.png)
引导扇区结构：
![alt text]./assets/bootsector.png)
读取引导扇区和FAT表
![alt text]./assets/readboot.png)
文件系统初始化：
![alt text]./assets/fatinit.png)

FAT_ClusterToLba:
![alt text]./assets/fatclustertolba.png)
FAT_OpenEntry:
![alt text]./assets/fatopenentry.png)

<br />

FAT_NextCluster:
![alt text]./assets/fatnextcluster.png)

<br />

FAT_Read:
![alt text]./assets/fat_read.png)
FAT_ReadEntry:
![alt text]./assets/fatreadentry.png)
FAT_Write:
![alt text]./assets/fatwrite.png)
FAT_FindFile
![alt text]./assets/fatfindfile.png)
FAT_Open:
![alt text]./assets/fatopen.png)

### <span> 4. </span> shellCmd.c
ls:
![alt text]./assets/commandls.png)
cd：
![alt text]./assets/commandcd.png)
cat:
![alt text]./assets/commandcat.png)
echo:
![alt text]./assets/commandecho.png)
转换文件名到fat格式的函数：
![alt text]./assets/tofat.png)
转换fat格式到文件名的函数
![alt text]./assets/toname.png)
## 实验演示
在stage2文件夹的main.c 测试。
### 根文件：
![alt text]./assets/roofile.png)

### 测试1
![alt text]./assets/firsttest.png)
![alt text]./assets/test1.png)
在这里演示的是cat 和 echo的命令，也就是读和写操作。从图中可看到文件内容从 “Original data from file!" 写成 "new data from text file"


### 测试2
![alt text]./assets/secondtest.png)
![alt text]./assets/test2.png)
使用cd 进入文件夹mydir，ls列出所有文件夹里面的文件，并cat 输出里面的文件的内容
### 测试3
之后我们测试一个大文本 （大小超过512 byte， 1 sector）的cat
调用：
![alt text]./assets/test3.png)


