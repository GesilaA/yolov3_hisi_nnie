# HISILICON NNIE YOLOV3 SAMPLE
基于海思3519的YOLOv3_NNIE例程
海思官方文档附带了很多目标检测算法的例子，但是在阅读的时候十分痛苦（我自己），函数各种跳跃；自带的Makefile也是一层一层嵌套，实在难以理解。所以单独将YOLOv3提取出来，并用CMakeLists替代层层嵌套的Makefile，方便阅读和将YOLOv3替换为其他网络。目前将海思例程中的YOLOv3分为两部分，一部分为通用的直接调用海思SDK的common部分，和YOLOv3特定的数据结构及方法。
萌新刚学海思，希望有海思大佬能提供一下其他网络的优化和部署。（海思好难啊）</p>
水平很菜，随缘更新。


## 1. 下载
```git clone https://github.com/gesilaa/yolov3_hisi_nnie.git```

## 2. 编译
我使用的是海思3519芯片，所以如果芯片不同的话，需要在CMakeLists.txt里面替换自己的交叉编译工具链。
```
export LC_ALL=C
cd yolov3_hisi_nnie
mkdir build; cd build
cmake..
make
cp ../run.sh ./
```

## 3. 运行
将工程复制到海思终端上
```
cd yolov3_hisi_nnie/build
sh ./run.sh
```
