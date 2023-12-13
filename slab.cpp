#include<pthread.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string>
using namespace std;
#define ops 5
#define spc 5
struct pname{
    int proc;
    struct pname *next;
};
struct Slab{
    
    int size;//每个slab中object
    int use;//当前slab中的object
    void *s_mem;//指向第一个object
    struct Slab* next;
    struct pname* list;//该slab所属obj的列表
};
struct list_head{
    struct Slab* head;
    struct Slab* tail;
};
struct cache{
    int total;//总占用obj，最多size*spc
    int num;//每个slab中object个数
    pthread_mutex_t mutex;//互斥锁
    struct list_head * slab_full;
    struct list_head * slab_partial;
    struct list_head * slab_empty;
    struct cache * next;
};

struct cache *_init(int size);
int allocate(struct cache*& c,int type,int want,int pro);
void allo_slab(struct cache* c,int pro);
void print(struct cache* c);
void free_slab(struct cache* c,int pro);
void free(struct cache* c,int type,int need,int pro);

struct cache * _init(int size){
    struct cache* head;
    struct cache* firstspace;
    firstspace = (cache*)malloc(sizeof(cache));
    head = firstspace;
    firstspace->next=nullptr;
    firstspace->num = size;
    firstspace->total = 0;
    //初始化三种队列
    firstspace->slab_full = (list_head*)malloc(sizeof(list_head));
    firstspace->slab_partial = (list_head*)malloc(sizeof(list_head));
    firstspace->slab_empty = (list_head*)malloc(sizeof(list_head));
    struct Slab* newslab1 = (Slab*)malloc(sizeof(Slab));
    newslab1->size = 0;
    newslab1->next = nullptr;
    firstspace->slab_full->head=newslab1;
    firstspace->slab_full->tail=newslab1;
    struct Slab* newslab2 = (Slab*)malloc(sizeof(Slab));
    newslab2->size = 0;
    newslab2->next = nullptr;
    firstspace->slab_partial->head=newslab2;
    firstspace->slab_partial->tail=newslab2;
    struct Slab* newslab3 = (Slab*)malloc(sizeof(Slab));
    newslab3->size = 0;
    newslab3->next = nullptr;
    firstspace->slab_empty->head=newslab3;
    firstspace->slab_empty->head=newslab3;
    struct pname* newp = (pname*)malloc(sizeof(pname));
    newp->next=nullptr;
    newp->proc=0;
    struct Slab* last,*next;
    last = newslab3;
    next = nullptr;
    for(int i=0;i<spc;i++){
        struct Slab* newslab = (Slab*)malloc(sizeof(Slab));
        newslab->size=size;
        newslab->use = 0;
        struct pname* newp = (pname*)malloc(sizeof(pname));
        newp->next=nullptr;
        newp->proc=0;
        newslab->list=newp;
        last->next = newslab;
        newslab->next=next;
        last=newslab;
        next=nullptr;
    }
    firstspace->slab_empty->tail=last;//完成empty列的初始化

    firstspace->num=size;
    return head;
}
void allo_slab(struct cache* c,int pro){
    if(c->slab_partial->head->next==nullptr){
        struct Slab* first = c->slab_empty->head->next;
        first->use++;
        // struct pname *p  = c->slab_empty->head->next->list;
        // struct pname *temp = p->next;
        // struct pname *newp = (pname*)malloc(sizeof(pname));
        // p->next=newp;
        // newp->next=temp;
        // newp->proc=pro;
        c->slab_empty->head->next=first->next;
        first->next=nullptr;
        //完成list的添加和empty的移除
        first->next = c->slab_partial->head->next;
        c->slab_partial->head->next=first;
        //完成将first移动到partial
        printf("cache %d 分配 obj in empty slab 给进程%d\n",c->num,pro);
        printf("cache %d 移动 1 empty slab 到 partial slab \n",c->num);
        
    }
    else{
        struct Slab* first = c->slab_partial->head->next;//定位到第一个partial slab
        first->use++;
        printf("cache %d 分配 obj in partial slab 给进程%d\n",c->num,pro);
        if(first->use==first->size){//此partial 满了
            c->slab_partial->head->next=first->next;
            first->next=NULL;
            struct Slab* ffull = c->slab_full->head;
            first->next = ffull->next;
            ffull->next = first;
            //完成将partial移动到full
            printf("cache %d 移动 1 partial slab 到 full slab \n",c->num);
        }
    }
}
int allocate(struct cache*& c,int type,int want,int pro){
    struct cache* t = c;
    while(t!=NULL){
        if(t->num!=type){//不是相同类型的slab
            t=t->next;
        }
        else{
            if(t->total+want>t->num*spc){//此cache满
                return 0;//分配失败
            }
            else{//可以分配obj
                t->total+=want;
                for(int i=0;i<want;i++){
                    allo_slab(t,pro);
                }
                print(c);
                return 1;
            }
        }
    }
    //请求新cache
    struct cache* newcache = _init(type);
    printf("请求cache %d\n",type);
    newcache->next = c;
    c = newcache;
    t=c;
    while(t!=NULL){
        t->total+=want;
        if(t->total>t->num*spc){
            t->total=0;
            return 0;
        }
        for(int i=0;i<want;i++){
            allo_slab(t,pro);
        }
        print(c);
        return 1;
    }
    return 0;
}
void free_slab(struct cache* c,int pro){
    struct Slab* t = c->slab_partial->head->next;
    if(t==NULL){//partial为空
        t=c->slab_full->head->next;
        if(t==NULL){//full为空
            printf("free_slab: full and partial is empty\n");
            return;
        }
        else{//从full移动到partial
            c->slab_full->head->next=t->next;
            t->use--;
            t->next=c->slab_partial->head->next;
            c->slab_partial->head->next=t;
            printf("cache %d 回收obj in full slab 从进程%d\n",c->num,pro);
            printf("cache %d 移动 1 full slab 到 partial slab \n",c->num);
        }
    }
    else{
        t->use--;
        printf("cache %d 回收 obj in partial slab 从进程%d\n",c->num,pro);
        if(t->use==0){//回收到empty队列中
            c->slab_partial->head->next=t->next;
            t->next=c->slab_empty->head->next;
            c->slab_empty->head->next=t;
            printf("cache %d 移动 1 partial slab 到 empty slab \n",c->num);
        }
    }
}
void free(struct cache* c,int type,int need,int pro){
    struct cache* t = c;
    while(t!=NULL){
        if(t->num!=type){//不是相同类型的slab
            t=t->next;
        }
        else{
            if(t->total-need<0){//此cache不足
                printf("free_slab: not enough\n");
                return;
            }
            else{//可以释放obj
                for(int i=0;i<need;i++){
                    free_slab(t,pro);
                }
                t->total-=need;
                print(c);
                return;
            }
        }
    }
}
void print(struct cache* c){
    while(c!=NULL){
        printf("-----cache %d total: %d+----full slab     :",c->num,c->total);
        struct Slab* tmp = c->slab_full->head->next;
        while(tmp!=nullptr){
            printf(" %d ",tmp->use);
            tmp=tmp->next;
        }
        printf("\n         |           |-----partial slab :");
        tmp = c->slab_partial->head->next;
        while(tmp!=nullptr){
            printf(" %d ",tmp->use);
            tmp=tmp->next;
        }
        printf("\n         |           +-----empty slab   :");
        tmp = c->slab_empty->head->next;
        while(tmp!=nullptr){
            printf(" %d ",tmp->use);
            tmp=tmp->next;
        }
        printf("\n         | \n");
        c=c->next;
    }
}

int main(){
    struct cache* head = _init(ops);
    allocate(head,4,13,1);

    return 0;
}