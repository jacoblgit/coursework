#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>
#include <float.h>
#include "kdtree.h"
#include "location.h"
#include "list.h"

#define LAT 0
#define LON 1
#define ROOT_CUTTING_DIM 0


typedef struct _node node;
struct _node {
    location* loc;
    node* par;
    node* left;
    node* right;
    int cutting_dim;
};

typedef struct {
    location sw;
    location ne;
} region;

struct _kdtree {
    node* root;
};

// static int min_int(int a, int b);
static node* find_min(node* n, int search_dim);
static node* find_max(node* n, int search_dim);
static node* min_node(node* a, node* b, int dim);
static node* max_node(node* a, node* b, int dim);
static node* create_helper(location **pts, node* p, int n, int dim);
static void destroy_helper(node* root);
static void remove_helper(node* n, kdtree* t);
static void range_for_each_helper(node* n, region r, region b, void (*f)(const location *, void *), void *arg);
static void range_helper(const location* loc, void* l);
static bool regionIsEmpty(region r);
static region get_intersection(region a, region b);
static bool isInRegion(location* loc, region r);
static double min_double(double a, double b);
static double max_double(double a, double b);
static bool coordsAreEqual(const location* loc1, const location* loc2);
static location* copyLocation(const location* loc);
static int compare_helper_dim_LAT(const void* loc1, const void* loc2);
static int compare_helper_dim_LON(const void* loc1, const void* loc2);
// static void print_points_array(location** pnts, int len);

/**
 * Creates a set of points in a balanced k-d tree containing copies of
 * the points in the given array of locations.  If n is 0 then the
 * returned tree is empty.  If the array contains multiple copies of
 * the same point (with "same" defined as described above), then only
 * one copy is included in the set.
 *
 * @param pts an array of valid locations; NULL is allowed if n = 0
 * @param n the number of points to add from the beginning of that array,
 * or 0 if pts is NULL
 * @return a pointer to the newly created set of points
 */
kdtree *kdtree_create(const location *pts, int n)
{
    int len;                        // number of unique points
    location** pts_cleaned;         // array of unique, valid points
    location *curr_loc, *prev_loc;

    // initalize new kdtree
    kdtree* new_tree = malloc(sizeof(*new_tree));
    new_tree->root = NULL;
    if (n == 0) return new_tree;

    // remove duplicate & invalid points
    // is there a better way of doing this?
    
    // COPY all points to new array of pointers
    pts_cleaned = malloc(sizeof(*pts_cleaned) * n);
    for (int i = 0; i < n; i++) {
        pts_cleaned[i] = copyLocation(&pts[i]);
    }
    
    // sort new array
    qsort(pts_cleaned, n, sizeof(*pts_cleaned), compare_helper_dim_LAT);
    
    len = 1;
    prev_loc = pts_cleaned[0];    
    // iterate through elements in pts array
    for (int i = 1; i < n; i++)
    {
        curr_loc = pts_cleaned[i];
        if (!coordsAreEqual(prev_loc, curr_loc))
        {
            assert(location_validate(curr_loc));
            // moves valid, unique points to appropriate index in "clean" array
            pts_cleaned[len] = curr_loc;
            len++;
        }
        prev_loc = curr_loc;
    }

    new_tree->root = create_helper(pts_cleaned, NULL, len, ROOT_CUTTING_DIM);
    free(pts_cleaned);

    return new_tree;
}

/**
 * Adds a copy of the given point to the given k-d tree.  There is no
 * effect if the point is already in the tree.  The tree need not be
 * balanced after the add.  The return value is true if the point was
 * added successfully and false otherwise (if the point was already in the
 * tree).
 *
 * @param t a pointer to a valid k-d tree, non-NULL
 * @param p a pointer to a valid location, non-NULL
 * @return true if and only if the point was successfully added
 */
bool kdtree_add(kdtree *t, const location *p) {
    node* par = NULL;       // parent to current node
    node* curr = t->root;   // current node
    int dim = ROOT_CUTTING_DIM;
    int compare;
    
    // descend through tree until leaf or target is found
    while (curr != NULL && !coordsAreEqual(curr->loc, p))
    {
        // curr is not a leaf nor the target
        
        par = curr;
        
        // identify appropriate subtree based on current dimension or comparison
        if (dim == LAT)
        {
            compare = location_compare_latitude(p, curr->loc);
        }
        else
        {
            compare = location_compare_longitude(p, curr->loc);
        }

        if(compare > 0) 
        {
            curr = curr->right;
        }
        else 
        {
            curr = curr->left;
        }

        dim = (dim + 1) % 2;
    }

    // par is a leaf (target node is not already in tree)
    if (curr == NULL)
    {
        // create new node
        node* new_node = malloc(sizeof(*new_node));
        new_node->par = par;
        new_node->cutting_dim = dim;
        new_node->loc = copyLocation(p);
        new_node->left = NULL;
        new_node->right = NULL;
        
        // check if tree is empty
        if (t->root == NULL) {
            t->root = new_node;
            return true;
        }

        // par should only ever equal NULL if the tree is empty
        assert(par != NULL);

        // identify appropriate insert location based on dimension of comparison
        if (par->cutting_dim == LAT)
        {
            compare = location_compare_latitude(p, par->loc);
        }
        else
        {
            compare = location_compare_longitude(p, par->loc);
        }

        if(compare > 0) 
        {
            par->right = new_node;
        }
        else 
        {
            par->left = new_node;
        }
        
        return true;
           
    }

    // point already existed in tree
    return false;
}

/**
 * Determines if the given tree contains a point with the same coordinates
 * as the given point.
 *
 * @param t a pointer to a valid k-d tree, non-NULL
 * @param p a pointer to a valid location, non-NULL
 * @return true if and only of the tree contains the location
 */
bool kdtree_contains(const kdtree *t, const location *p)
{
    assert(t != NULL);
    assert(p != NULL);

    node* curr = t->root;   // current node
    int dim = ROOT_CUTTING_DIM;
    int compare;

    // descend through tree until leaf or target is found
    while (curr != NULL && !coordsAreEqual(curr->loc, p))
    {
        // curr is not a leaf nor the target
                
        // identify appropriate subtree based on current dimension or comparison
        if (dim == LAT)
        {
            compare = location_compare_latitude(p, curr->loc);
            dim = LON;
        }
        else if (dim == LON)
        {
            compare = location_compare_longitude(p, curr->loc);
            dim = LAT;
        }
        else
        {
            exit(EXIT_FAILURE);
        }

        if(compare > 0) 
        {
            curr = curr->right;
        }
        else 
        {
            curr = curr->left;
        }
    }

    // par is a leaf (target node is not already in tree)
    if (curr == NULL) return false;

    // otherwise, point already existed in tree
    return true; 
}

/**
 * Removes the point with the coordinates as the given point
 * from this k-d tree.  The tree need not be balanced
 * after the removal.  There is no effect if the point is not in the tree.
 *
 * @param t a pointer to a valid k-d tree, non-NULL
 * @param p a pointer to a valid location, non-NULL
 */
void kdtree_remove(kdtree *t, const location *p)
{
    node* curr = t->root;   // current node
    int dim = LAT;
    int compare;
    
    // descend through tree until leaf or target is found
    while (curr != NULL && !coordsAreEqual(curr->loc, p))
    {        
        // identify appropriate subtree based on current dimension or comparison
        if (dim == LAT)
        {
            compare = location_compare_latitude(p, curr->loc);
            dim = LON;
        }
        else if (dim == LON)
        {
            compare = location_compare_longitude(p, curr->loc);
            dim = LAT;
        }
        else
        {
            exit(EXIT_FAILURE);
        }

        if(compare > 0) 
        {
            curr = curr->right;
        }
        else 
        {
            curr = curr->left;
        }
    }

    // par is a leaf (target node is not already in tree)
    if (curr == NULL) return;

    // otherwise, point exists in tree
    remove_helper(curr, t);

    return;    
}

/**
 * Passes the points in the given tree that are in or on the borders of the
 * (spherical) rectangle defined by the given corners to the given function
 * in an arbitrary order.  The last argument to this function is also passed
 * to the given function along with each point.
 *
 * @param t a pointer to a valid k-d tree, non-NULL
 * @param sw a pointer to a valid location, non-NULL
 * @param ne a pointer to a valid location with latitude and longitude
 * both strictly greater than those in sw, non-NULL
 * @param f a pointer to a function that takes a location and
 * the extra argument arg, non-NULL
 * @param arg a pointer to be passed as the extra argument to f
 */
void kdtree_range_for_each(const kdtree *t, const location *sw,
                           const location *ne,void (*f)(const location *, void *), void *arg) {
    region r;
    r.ne = *ne;
    r.sw = *sw;
    
    region b; 
    location b_ne = {DBL_MAX, DBL_MAX};
    location b_sw = {DBL_MIN, DBL_MIN};
    b.ne = b_ne;
    b.sw = b_sw;

    range_for_each_helper(t->root, r, b, f, arg);
}


/**
 * Returns a dynamically allocated array containing the points in the
 * given tree in or on the borders of the (spherical) rectangle
 * defined by the given corners and sets the integer given as a
 * reference parameter to its size.  The points may be stored in the
 * array in an arbitrary order.  If there are no points in the
 * region, then the returned array may be empty, or it may be NULL.
 * It is the caller's responsibility ensure that the returned array
 * is eventually freed if it is not NULL.
 *
 * @param t a pointer to a valid k-d tree, non-NULL
 * @param sw a pointer to a valid location, non-NULL
 * @param ne a pointer to a valid location with latitude and longitude
 * both strictly greater than those in sw, non-NULL
 * @param n a pointer to an integer, non-NULL
 * @return a pointer to an array containing the points in the range, or NULL
 */
location *kdtree_range(const kdtree *t, const location *sw, const location *ne, int *n)
{
    list* l = list_create();
    kdtree_range_for_each(t, sw, ne, range_helper, l);

    *n = list_size(l);
    location* range = list_copy_data_array(l);
    list_destroy(l);

    return range;
}

static void range_helper(const location* loc, void* l)
{
    list* range = (list*) l;
    list_add(range, *loc);
    return;
}

// n, current node
// r: target region
// b: possible range of values in n's subtree
static void range_for_each_helper(node* n, region r, region b, void (*f)(const location *, void *), void *arg)
{
    if (n == NULL || regionIsEmpty(get_intersection(r, b))) return;

    if (isInRegion(n->loc, r)) {
        f(n->loc, arg);
    }

    region left_b = b;
    region right_b = b;

    if (n->cutting_dim == LAT)
    {
        left_b.ne.lat = n->loc->lat;
        right_b.sw.lat = n->loc->lat;
    }
    else /* n->cutting_dim == LON */
    {
        left_b.ne.lon = n->loc->lon;
        right_b.sw.lon = n->loc->lon;
    }

    range_for_each_helper(n->left, r, left_b, f, arg);
    range_for_each_helper(n->right, r, right_b, f, arg);
    return;
}

// returns true iff the region is an empty set
static bool regionIsEmpty(region r) {
    double west = r.sw.lon;
    double south = r.sw.lat;
    double east = r.ne.lon;
    double north = r.ne.lat;

    if (west > east || south > north) return true;
    else return false;
}

// returns a region that represents the interesection
// between the given ones
static region get_intersection(region a, region b)
{
    region intersection;

    double west = max_double(a.sw.lon, b.sw.lon);
    double south = max_double(a.sw.lat, b.sw.lat);
    double east = min_double(a.ne.lon, b.ne.lon);
    double north = min_double(a.ne.lat, b.ne.lat);

    intersection.sw.lon = west;
    intersection.sw.lat = south;
    intersection.ne.lon = east;
    intersection.ne.lat = north;

    return intersection;
}

static bool isInRegion(location* loc, region r)
{
    if (regionIsEmpty(r)) return false;

    else if (location_compare_longitude(loc, &r.sw) < 0
             || location_compare_latitude(loc, &r.sw) < 0
             || location_compare_longitude(loc, &r.ne) > 0
             || location_compare_latitude(loc, &r.ne) > 0) {
                return false;
             }
    else return true;
}

// n is node to delete
static void remove_helper(node* n, kdtree* t)
{
    // if curr is a leaf, remove it
    if (n->left == NULL && n->right == NULL)
    {
        if (n->par == NULL) {
            /* n is root node */
            t->root = NULL;
        }
        else if (n->par->left == n) {
            n->par->left = NULL;
        }
        else {
            n->par->right = NULL;
        }

        free(n->loc);
        free(n);
        return;
    }
    else if (n->right != NULL)
    {
        // find node, minimum, w/ lowest value with
        // respect to n's cutting dim in n's right subtree
        node* minimum = find_min(n->right, n->cutting_dim);
        
        // do the swap
        free(n->loc);
        n->loc = copyLocation(minimum->loc);
        
        // recurse
        remove_helper(minimum, t);
    }
    else /* n->left != NULL */
    { 
        // find node, maximum, w/ highest value with
        // respect to n's cutting dim in n's left subtree
        node* maximum = find_max(n->left, n->cutting_dim);

        // do the swap
        free(n->loc);
        n->loc = copyLocation(maximum->loc);

        // recurse
        remove_helper(maximum, t);
    }
}

// find minimum value node in n's subtree
// search_dim is dimension of minimum of interest
static node* find_min(node* n, int search_dim) {
    if (n == NULL) return NULL;
    
    if (n->cutting_dim == search_dim) {
        if (n->left != NULL) {
            return find_min(n->left, search_dim);
        }
        else {
            return n;
        }
    } else
    {        
        node* minimum = min_node(n, find_min(n->left, search_dim), search_dim);
        minimum = min_node(minimum, find_min(n->right, search_dim), search_dim);

        return minimum;
    }
}

// find maximum value node in n's subtree
// search_dim is dimension of minimum of interest
static node* find_max(node* n, int search_dim) {
    if (n == NULL) return NULL;
    
    if (n->cutting_dim == search_dim) {
        if (n->right != NULL) {
            return find_max(n->right, search_dim);
        }
        else {
            return n;
        }
    } else
    {        
        node* maximum = max_node(n, find_max(n->left, search_dim), search_dim);
        maximum = max_node(maximum, find_max(n->right, search_dim), search_dim);

        return maximum;
    }
}

/**
 * Destroys the given k-d tree.  The tree is invalid after being destroyed.
 *
 * @param t a pointer to a valid k-d tree, non-NULL
 */
void kdtree_destroy(kdtree *t)
{
    destroy_helper(t->root);
    free(t);
}

// dim encodes the dimension (0:lat, or 1:lon) to split by
// n encodes number of points in pts array
// returns root of kd subtree. Caller takes ownership
// p, parent node, is the node that called the create_helper funciton
static node* create_helper(location **pts, node* p, int n, int dim)
{
    // basecase (no elements in pts array)
    if (n == 0) return NULL;
    assert(n > 0);

    // identify median point with respect to dim (0:lat, or 1:lon)
    if (dim == LAT) {
        qsort(pts, n, sizeof(*pts), compare_helper_dim_LAT);

        // printf("sorted by lat:\n");
        // print_points_array(pts, n);
    }
    else
    {
        qsort(pts, n, sizeof(*pts), compare_helper_dim_LON);

        // printf("sorted by lon:\n");
        // print_points_array(pts, n);
    }

    // is there a better way to pick the median element?
    // current method splits even-length arrays into larger left, and smaller right
    int mid_index = n / 2;
    
    // initialize new node
    node* root = malloc(sizeof(*root));
    
    root->loc = pts[mid_index];
    assert(location_validate(root->loc));
    root->par = p;
    root->cutting_dim = dim;

    // testing
    // location test = {24.904359601287595, -164.679680919231197};
    // if (coordsAreEqual(&test, root->loc)) printf("node added\n");

    // recursive calls to fill in children
    int nextDim = (dim + 1) % 2;
    int nLeft = n / 2;
    int nRight = n - 1 - nLeft;

    root->left = create_helper(pts, root, nLeft, nextDim);
    // root->right = create_helper(&(pts[min_int(mid_index + 1, n - 1)]), root, nRight, nextDim);
    root->right = create_helper(pts + mid_index + 1, root, nRight, nextDim);

    return root;
}

static void destroy_helper(node* root) {
    // base case
    if (root == NULL) return;

    destroy_helper(root->left);
    destroy_helper(root->right);

    // printf("(%lf, %lf)\n", root->loc->lat, root->loc->lon);
    free(root->loc);
    free(root);

    return;
}

// returns the minimum of the integer arguements
// static int min_int(int a, int b) {
//     if (a < b) return a;
//     else return b;
// }

static double min_double(double a, double b) {
    if (a < b) return a;
    else return b;
}
static double max_double(double a, double b) {
    if (a < b) return b;
    else return b;
}

// returns the minimum of the node arguments w/ respect to dimension dim
static node* min_node(node* a, node* b, int dim) {
    assert(dim == LAT || dim == LON);

    if (a == NULL && b == NULL) return NULL;
    else if (a == NULL) return b;
    else if (b == NULL) return a;
    
    if (dim == LAT)
    {
        if (location_compare_latitude(a->loc, b->loc) > 0) return b;
        else return a;
    }
    else /* (dim == LON) */
    {
        if (location_compare_longitude(a->loc, b->loc) > 0) return b;
        else return a;
    }

}

// returns the maximum of the node arguments w/ respect to dimension dim
static node* max_node(node* a, node* b, int dim) {
    assert(dim == LAT || dim == LON);
    
    if (a == NULL && b == NULL) return NULL;
    else if (a == NULL) return b;
    else if (b == NULL) return a;
    
    if (dim == LAT)
    {
        if (location_compare_latitude(a->loc, b->loc) > 0) return a;
        else return b;
    }
    else /* (dim == LON) */
    {
        if (location_compare_longitude(a->loc, b->loc) > 0) return a;
        else return b;
    }
}


// returns true if the lat and lon if the given points are identical
static bool coordsAreEqual(const location* loc1, const location* loc2) {
    return ((loc1->lat == loc2->lat) && (loc1->lon == loc2->lon));
}

// returns a pointer to a copy of the given location
// caller takes ownership of returned location
static location* copyLocation(const location* loc)
{
    location* new_loc = malloc(sizeof(*new_loc));
    new_loc->lat = loc->lat;
    new_loc->lon = loc->lon;
    return new_loc;
}

static int compare_helper_dim_LAT(const void* loc1, const void* loc2) {
    return location_compare_latitude( *((location**) loc1), *((location**) loc2));
}

static int compare_helper_dim_LON(const void* loc1, const void* loc2) {
    return location_compare_longitude( *((location**) loc1), *((location**) loc2));
}

// static void print_points_array(location** pnts, int len) {
//     for (int i = 0; i < len; i++) {
//         printf("(%5.3lf, %5.3lf)\n", pnts[i]->lat, pnts[i]->lon);
//     }
//     printf("\n\n");
// }