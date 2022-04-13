
#define RED 0
#define BLACK 1

typedef int KEY_TYPE;

int  key_compare (KEY_TYPE *a, KEY_TYPE *b)
{
    
}

//红黑树节点
typedef struct _rbtree_node
{
    unsigned char color;
    struct _rbtree_node *left;      //左子树
    struct _rbtree_node *right;     //右子树
    struct _rbtree_node *parent;    //父节点，旋转用

    KEY_TYPE key;                   //用于对比，通过key值查找
    void *value;

}rbtree_node;

typedef struct _rbtree 
{
    rbtree_node *root;  //根节点
    rbtree_node *nil;   //通用节点，用于判断叶子节点（所有叶子节点都指向nil，若不指向nil则不是叶子节点）
}rbtree;

/********以上是红黑树的定义***********/

//左旋
void _left_rotate(rbtree *T, rbtree_node *x)
{
    rbtree_node *y = x->right; //条件

    // 1 
    x->right = y->left;
    if (y->left != T->nil)
    {
        y->left->parent = x;
    }

    // 2
    y->parent = x->parent;
    if (x->parent == T->nil)
    {
        T->root = y;
    } else if (x == x->parent->left)
    {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    
    // 3
    y->left = x;
    x->parent = y;
    
}


void rbtree_insert_fixup(rbtree *T, rbtree_node *z)
{
    while (z->parent->color == RED)
    {
        if (z->parent == z->parent->parent->left)
        {
            rbtree_node *y = z->parent->parent->right;

            if (y->color == RED)
            {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;

                z = z->parent->parent;
            } else
            {
                if (z == z->parent->right)
                {
                    z = z->parent;
                    _left_rotate(T, z);
                }
                
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                _right_rotate(T, z->parent->parent);
            }
        }
    }
    T->root->color = BLACK;   
}


//插入
void rbtree_insert(rbtree *T, rbtree_node *z)
{
    rbtree_node *x = T->root;
    rbtree_node *y = T->nil;

    while (x != T->nil)
    {
        y = x;
        if (z->key < x->key)
        {
            x = x->left;
        } else if (z->key > x->key)
        {
            x = x->right;
        } else
        {
            return ;
        }
    }

    z->parent = y;
    if (y == T->nil)
    {
        T->root = z;
    } else if (z->key < y->key)
    {
        y->left = z;
    } else
    {
        y->right = z;
    }
    
    z->left = T->nil;
    z->right = T->nil;

    z->color = RED;
 
    
}

