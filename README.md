# MeshSimplificationFramework

首先Greedy系列直接压缩即可，无需在意其他

在fast系列中，第一个需要引入error计算方法，然后就是第二个问题，如何遍历？

可以使用图的遍历方法，一轮接一轮的遍历

为每个点都设置一个processed flag，这样防止每个点被处理多次
 
多次遍历，每次遍历时的quardic阈值都会出现变化，直到不能遍历为止


一直坚持unified model和unified storage原则，unified storage，bit记录需处理的 + 

本质是移除点 + 如何三角化的问题
