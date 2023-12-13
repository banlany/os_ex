#include<stdio.h>
#include<stdlib.h>
#include<random>
using namespace std;
#define FULL 2048
#define cold 0
#define hot 1
#define hotpage_size 64
int hot_c =0;
struct Frame{
    int size;
    int use;
    int flag;
    int type;
    Frame* next;
};
struct Frame* init();
int allocate(struct Frame *cs,int want);
void free(struct Frame *cs,int want);
void print(struct Frame *cs);

 struct Frame* init(){
    struct Frame *head;
    struct Frame* nextspace;
    nextspace=(Frame*)malloc(sizeof(Frame));
    head=nextspace;
    nextspace->size=FULL;
    nextspace->use=0;
    nextspace->flag=0;
    nextspace->type=cold;
    nextspace->next=NULL;
    return head;
 }
 int allocate(struct Frame *cs,int want){
    struct Frame* t=cs;
    while(t!=NULL&&t->type==hot){
        if(t->size >= want){
            printf("in hotpage need: %d,find: %d,flag: %d\n", want,t->size,t->flag);
            print(cs);
            if(t->flag==0){
                if(t->size/2 < want){ //不用分割
                    printf("want:%d succeed\n",want);
                    t->use=want;
                    t->flag=1;
                    return 1;
                }
                else{
                    struct Frame* temp = t->next;
                    struct Frame* buddyspace;
                    buddyspace=(Frame*)malloc(sizeof(Frame));
                    buddyspace->size=t->size/2;
                    buddyspace->use=0;
                    buddyspace->flag=0;
                    buddyspace->type=hot;
                    hot_c++;
                    t->size=buddyspace->size;//均分空间
                    t->next = buddyspace;//将一个空间分为两个链表元素
                    buddyspace->next = temp;
                    allocate(t,want);//分配一半的空间
                    return 1;
                }
            }
            else{
                t=t->next;
            }
        }
        else{
            t=t->next;
        }
    }
    while(t!=NULL){
        if(t->size >= want){
            printf(" need: %d,find: %d\n", want,t->size);
            print(cs);
            if(t->flag==0){
                if(t->size/2 < want){ //不用分割
                    printf("want:%d succeed\n",want);
                    t->use=want;
                    t->flag=1;
                    return 1;
                }
                else{
                    struct Frame* temp = t->next;
                    struct Frame* buddyspace;
                    buddyspace=(Frame*)malloc(sizeof(Frame));
                    buddyspace->size=t->size/2;
                    buddyspace->use=0;
                    buddyspace->flag=0;
                    if(hot_c<=5&&t->type==cold&&t->size<=hotpage_size){
                        t->type=hot;
                        hot_c++;
                    }
                    buddyspace->type=cold;
                    t->size=buddyspace->size;//均分空间
                    t->next = buddyspace;//将一个空间分为两个链表元素
                    buddyspace->next = temp;
                    allocate(t,want);//分配一半的空间
                    return 1;
                }
            }
            else{
                t=t->next;
            }
        }
        else{
            t=t->next;
        }
    }
    return 0;//未能成功分配,没有充足空间
 }
 void free(struct Frame *cs,int want){
    printf("开始释放:%d\n",want);
    struct Frame* t=cs;
    while(t!=NULL){
        if(t->use==want){
            t->flag=0;
            t->use=0;
            if(hot_c>5){
                if(t->type==hot){
                    t->type=cold;
                    hot_c--;
                }
            }
            break;
        }
        t=t->next;
    }
    t = cs;
    struct Frame* tn = cs->next;
    while(tn!=NULL){
        if(t->size== tn->size){
            if(t->flag==0 && tn->flag==0&&t->type==cold&&tn->type==cold){//都未使用，则合并
                t->next = tn->next;
                t->size = t->size + tn->size;
                free(tn);
                tn = t->next;
                print(cs);
            }
            else{
                t = t->next;
                tn = tn->next;
            }
        }
        else{
            t = t->next;
            tn = tn->next;
        }
    }
 }
 void print(struct Frame *head){
    struct Frame* t=head;
    while(t!=NULL){
        printf(" [use: %d  size: %d]",t->use,t->size);
        t=t->next;
    }
    printf("\n");

 }
 int getrand(int max){
     return rand()%max;
 }
int main(){
    struct Frame *head ;
    int c_successd=0,c_fail=0;
    int q[10];
    for(int i=0;i<10;i++){
        int s=getrand(11);
        q[i]=(1<<s)+getrand(22);
        printf("q[%d]:%d\n",i,q[i]);
    }
    head = init();
    print(head);
    for(int i=0;i<10;i++){
        int want = q[i];
        int get = allocate(head,want);
        if(get){
            print(head);
            c_successd++;
        }
        else{
            printf("!!!!want:%d failed",want);
            c_fail++;
        }
    }
    for(int i=0;i<10;i++){
        free(head,q[i]);
    }
    printf("successd:%d  failed:%d\n",c_successd,c_fail);
    return 0;
}