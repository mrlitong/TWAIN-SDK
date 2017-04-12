# <font color = #ff0000>TWAIN-Specification-CHN</font>

##TWAIN<font size=2>[维基百科链接](https://en.wikipedia.org/wiki/TWAIN "维基百科-TWAIN")</font>

TWAIN（全写：Technology Without An Interesting Name）是一个软件和数码相机、扫描仪等图像输入设备之间的通讯标准。  
TWAIN工作组于1990年组成，包含柯达、惠普、罗技等图像设备厂商和Aldus、Caerre等图像软件厂商。  
这个组织的目标是创建一个满足如下条件的标准：  

- 多平台支持
- 兼容多种设备
- 广泛的软硬件支持
- 可扩展性
- 针对最终用户和软件开发人员的易用性
- 多种图像格式支持  

从硬件到软件，TWAIN包含四层：硬件、源、源管理器和软件。硬件厂家的TWAIN支持通常体现为支持TWAIN接口的驱动程序。  
TWAIN的硬件层接口被称为源，源管理器负责选择和管理来自不同硬件厂家的源。在微软的Windows上，源管理器是以DLL方式实现。TWAIN软件不直接调用硬件厂家的TWAIN接口，而是通过源管理器。  
用户在TWAIN软件中选择获取图像之后，TWAIN软件和硬件通过一系列交涉来决定如何传输数据。软件描述它需要的图像，而硬件描述它能够提供的图像。  
如果软硬件在图像格式上达成一致，那么控制被传递到源。源现在可以设置扫描选项，以及开始扫描。  

----------
##说明
本项目详细阐述了TWAIN的重要组成部分：  

- TWAIN协议原理
- 应用程序层
- 源实现层
- Triplets部分(<font color="red">个人认为非常优秀的接口设计思想）</font>
- 数据类型及数据结构
- Capability参数说明（<font color="red">与应用程序开发关系最为密切的部分)</font>
- 返回码和错误码

----------

![](/specImg/TWAIN.png)
<p align = "center">TWAIN组成</p>

----------
![](/capability.png)
<p align = "center">参数说明</p>

----------
