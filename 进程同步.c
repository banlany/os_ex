// 6.7
do
{
    // 初始化等待列表
    waiting[i] = TRUE;
    // 尝试获取锁
    key = TRUE;
    while (waiting[i] && key)
        key = Swap(&lock, &key);
    // 获取锁成功，开始执行临界区代码
    waiting[i] = FALSE;
    // 尝试获取下一个锁
    j = (i + 1) % n;
    while ((j != i) && !waiting[j])
        j = (j + 1) % n;
    // 如果获取成功，则设置锁为false
    if (j == i)
        lock = FALSE;
    else
        waiting[j] = FALSE;
    // 等待
} while (TRUE);

// 6.10
//互斥锁
int guard = 0;
int value = 0;
wait()
{
    while (TestAndSet(&guard) == 1);
    if (value == 0)//信号量为0
    {
        add process to the wait queue;
    }
    else
    {
        //取一个值
       value--;
       
       guard = 0;
    }
}
signal()
{
    while (TestAndSet(&guard) == 1);
    if (value == 0 && there is a process on the wait queue)
    wake up the first process on the wait queue;
    else
    {   //放一个值
        value++;
        guard = 0;
    }
}

//6.13
monitor Demo{
    int items[MAX_ITEMS];
    int numItems = 0;
    condition full,empty;
    void producer(int v)
    {
        if( numItems == MAX_ITEMS)
        full.wait();
        items[numItems++]= v;
        empty.signal();
    }
    int consumer(){
        int reVal;
        while(numItems == 0){
            empty.wait();

        }
        reVal = items[--numItems];
        full.signal();
        return reVal;
    }
}
