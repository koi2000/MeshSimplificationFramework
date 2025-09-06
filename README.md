<!--
 * @Author: koi
 * @Date: 2025-03-19 16:25:19
 * @Description: 
-->
# MeshSimplificationFramework

需要完成的内容

## selection

可以统一化为一个优先级队列，当不需要带error的时候就退化为队列

## elimination

现在抽象为一个operator了，其实很简单，可以分为 erase_center_vertex 和 triagulation。然后可能会再调整点的位置。

在存储时也可以进行统一化，根据边变为bitmap去存储 然后根据面变为bitmap再存储。

## segmentation

在其中有另一个需要注意的问题，就是边界的处理问题


## compress

## decompress