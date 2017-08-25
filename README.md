## TWAIN协议介绍

> TWAIN（全写：Technology Without An Interesting Name）是一个软件和数码相机、扫描仪等图像输入设备之间的通讯标准。此标准如下：
> - 多平台支持
> - 兼容多种设备
> - 广泛的软硬件支持
> - 可拓展性
> - 易用性
> - 多图像格式支持
>
> 从硬件到软件，TWAIN包含四层：硬件、源、源管理器和软件。硬件厂家的TWAIN支持通常体现为支持TWAIN接口的驱动程序。TWAIN的硬件层接口被称为源，源管理器负责选择和管理来自不同硬件厂家的源。在微软的Windows上，源管理器是以DLL方式实现。TWAIN软件不直接调用硬件厂家的TWAIN接口，而是通过源管理器。用户在TWAIN软件中选择获取图像之后，TWAIN软件和硬件通过一系列交涉来决定如何传输数据。软件描述它需要的图像，而硬件描述它能够提供的图像。如果软硬件在图像格式上达成一致，那么控制被传递到源。源现在可以设置扫描选项，以及开始扫描。
> [https://zh.wikipedia.org/wiki/TWAIN](https://zh.wikipedia.org/wiki/TWAIN "https://zh.wikipedia.org/wiki/TWAIN")

## TWAIN结构
![](https://github.com/mrlitong/TWAIN-SDK/blob/master/source/twain_layout.jpg)

在该图中可以看到Source Manager作为Application与Source沟通的桥梁起着非常重要的作用，通过统一入口函数DSM_Entry(),我们可以在Application和Source之间传递消息，从而达到通讯的目的。而Source Manager与Source的通讯通过DS_Entry(）来进行，开发者不必也无需关心此函数如何被调用。




## What's TWAIN-SDK?
As you can see above introduction of TWAIN,which is just a protocol bewteen application and digital imaging devices.TWAIN will not be able to help you with digital imaging based application development.So,here is a cross-platform SDK that will enable you to develop your application without consideration about how different interfaces are.


