# 内存管理实验

## buddy

源码为buddy.cpp

+ 使用链表进行内存管理，根据buddy算法，可以形象显示内存的分配与释放

+ 热页设定为最后访问的5页是热页，热页在释放后不会进行合并，长时间没有访问则热页会转化为冷页

  ![image-20231213164451079](C:\Users\22230\AppData\Roaming\Typora\typora-user-images\image-20231213164451079.png)

  ![image-20231213164531901](C:\Users\22230\AppData\Roaming\Typora\typora-user-images\image-20231213164531901.png)

  此外，程序可以打印请求分割的页是冷还是热，访问无法得到满足会报！！！！

![image-20231213164724110](C:\Users\22230\AppData\Roaming\Typora\typora-user-images\image-20231213164724110.png)

释放时有内存合并过程以及结束后进行统计信息打印。

## slab

源代码为 slab1.cpp

+ 使用链表进行slab的分配，使用下图的结构

  ![image-20231213164957928](C:\Users\22230\AppData\Roaming\Typora\typora-user-images\image-20231213164957928.png)

![image-20231213165225333](C:\Users\22230\AppData\Roaming\Typora\typora-user-images\image-20231213165225333.png)

+ 通过对slab在三条队列的调度实现内存的分配与回收

+ 在请求新的obj种类时会请求新的cache放入链表

+ 通过pname链表记录slab中存在的进程obj，释放时做到精准释放

  ![image-20231213165558354](C:\Users\22230\AppData\Roaming\Typora\typora-user-images\image-20231213165558354.png)

  请求与移动时会打印信息，操作失败会有打印，进程操作结束时打印当前cache链表中结构。

  ![image-20231213165719570](C:\Users\22230\AppData\Roaming\Typora\typora-user-images\image-20231213165719570.png)

![image-20231213165817135](C:\Users\22230\AppData\Roaming\Typora\typora-user-images\image-20231213165817135.png)

内存回收后empty队列管理slab，实现内存管理，所有线程结束后打印统计信息。