# UCI2 User's guide

### The building blocks of UCI2 configuration system

#### AST parser context
Every configuration file, once read from the filesystem, gets parsed and converted to AST parser context. The tree itself uses a system of reference counting, and all nodes are kept track of. When node based tree structure is no longer needed, all nodes comprising the loaded configuration file are freed by invoking the`uci2_free_ctx`  method. Context based parsing, unlike global variables, enables users to work on multiple files simultaneously. UCI2 library was built from the ground up with [**reentrancy**](https://en.wikipedia.org/wiki/Reentrancy_(computing)) in mind.

#### Abstract Syntax Tree (AST)
[OpenWrt's configuration file syntax](https://openwrt.org/docs/guide-user/base-system/uci#file_syntax) consists of simple line based structures grouped by some keywords, some which are **config** and **list**. UCI2 library converts this flat file viewpoint to a more appropriate and useful form, a tree based structure also known as [AST](https://en.wikipedia.org/wiki/Abstract_syntax_tree).

#### AST nodes
As mentioned earlier, configuration comprise only a few node types:

* **UCI_NT_CFG_GROUP [@C]**
    * Root configuration node
* **UCI_NT_TYPE [T]**
    * Unnamed Type node (in terms of *OpenWrt*, this would represent an [**Unnamed Section**](https://openwrt.org/docs/guide-user/base-system/uci#uci_dataobject_model)
* **UCI_NT_SECTION_NAME [S]**
    * Named Type node, always positioned as a child of **[T]** node (in terms of *OpenWrt*, this would represent a [**Named Section**](https://openwrt.org/docs/guide-user/base-system/uci#uci_dataobject_model)
* **UCI_NT_OPTION [O]**
    * Option node comprising **Name = Value** syntax, a direct correlation to [**OpenWrt's option lines**](https://openwrt.org/docs/guide-user/base-system/uci#uci_dataobject_model)
* **UCI_NT_LIST [L]**
    * List node is an array of arbitrary values, a direct correlation to [**OpenWrt's list lines**](https://openwrt.org/docs/guide-user/base-system/uci#uci_dataobject_model)
* **UCI_NT_LIST_ITEM [I]**
    * List item node is a member, or a direct child of **[L]** List node. It correlates to the value part of   [**OpenWrt's list lines**](https://openwrt.org/docs/guide-user/base-system/uci#uci_dataobject_model)


### Building UCI2
cd /path/to/uci2/
mkdir build && cd build
cmake ..

### Enabling Tests with build
cd /path/to/uci2/
mkdir build && cd build
cmake -DENABLE_TESTS=ON ..

### Installing UCI2
sudo make install

### Configuration file handling
Original **/etc/system configuration** file:
```
config system
	option hostname 'OpenWrt'
	option timezone 'UTC'
	option ttylogin '0'
	option log_size '64'
	option urandom_seed '0'

config timeserver 'ntp'
	option enabled '1'
	option enable_server '0'
	list server '0.openwrt.pool.ntp.org'
	list server '1.openwrt.pool.ntp.org'
	list server '2.openwrt.pool.ntp.org'
	list server '3.openwrt.pool.ntp.org'

```


Internal **AST** representation used by **libuci2**:
```
[/]
  [@C]
    [T] system
      [O] hostname = OpenWrt
      [O] timezone = UTC
      [O] ttylogin = 0
      [O] log_size = 64
      [O] urandom_seed = 0
    [T] timeserver
      [S] ntp
        [O] enabled = 1
        [O] enable_server = 0
        [L] server
          [I] 0.openwrt.pool.ntp.org
          [I] 1.openwrt.pool.ntp.org
          [I] 2.openwrt.pool.ntp.org
          [I] 3.openwrt.pool.ntp.org
```

#### Loading configuration files from filesystem
The example presented here creates parser context by invoking `uci2_parse_file(fp)` method. As mentioned earlier, this method parses the file and converts it to AST. The final line, `uci2_free_ctx(ctx)`, releases the memory used up by the internal structure of nodes.
```c
// create context for file
uci2_parser_ctx_t* ctx = uci2_parse_file(fp);
// err check
if(!ctx) return 1;
...
...
...
// free context
uci2_free_ctx(ctx);
```



#### Creating new configuration structure from scratch
Unlike previous example, the following case assumes the absence of configuration file on the file system and generating one from scratch is the only option.  Both examples are quite similar, the only difference is the `uci2_new_ctx()` method which doesn't require any arguments, but creates the internal structure and prepares it for further addition of various nodes.
```c
// create context for file
uci2_parser_ctx_t* ctx = uci2_new_ctx();
// err check
if(!ctx) return 1;
...
...
...
// free context
uci2_free_ctx(ctx);
```

#### Exporting AST structure to destination configuration file
```c
// create context (uci2_new_ctx or uci2_parse_file)
...
// err check
if(!ctx) return 1;
...
...
...
// write context to file, make sure fsync is called
// this example writes dhcp configuration
uci2_export_ctx_fsync(ctx, "/etc/dhcp");
// free context
uci2_free_ctx(ctx);
```


#### Traversing, querying and modifying the Abstract Syntax Tree (AST)
* `uci2_n_t* UCI2_CFG_ROOT(ctx)`
    *  Returns a pointer to root configuration node (**[@C]**). This is the root node of all currently supported configuration nodes like types, section names, options, lists and list items.

* `uci2_n_t* uci2_q(ctx, ...)`
    * This method accepts arbitrary number of arguments which are used to traverse the tree of nodes. When argument is matched, the following one is matched against the list of child nodes of the previously matched node, advancing deeper into the AST. For example, to get the **ipaddr** of the **loopback interface** depicted by the following configuration lines:

```bash
 config interface 'loopback'
	option ifname 'lo'
	option proto 'static'
	option ipaddr '127.0.0.1'
	option netmask '255.0.0.0'
```
One would invoke the query method using the following syntax: 
`uci2_n_t* n = uci2_q(ctx, "interface", "loopback", "ipaddr")`

This would return a pointer to **ipaddr** option **[O]** node, and the value of **ipaddr** option would be available as a **name** member of **uci2_n_t** struct pointer (**n->name**).

When dealing with **unnamed sections**, an **auto-generated** name can be used to fetch a particular node in an array of sibling nodes.

`uci2_n_t* n = uci2_q(ctx, "rule", "@[1])`

This would return a pointer to the **2nd** unnamed **UCI_NT_TYPE** node (the one with **name** option set to **Allow-Ping**)


```bash
config rule
	option name		Allow-DHCP-Renew
	option src		wan
	option proto		udp
	option dest_port	68
	option target		ACCEPT
	option family		ipv4

config rule
	option name		Allow-Ping
	option src		wan
	option proto		icmp
	option icmp_type	echo-request
	option family		ipv4
	option target		ACCEPT

```
	 
* `char *uci2_get_value(n)`
    *  This method returns the **value** of node **n**. The only node type that uses the **value** part of node structure is the **options [0]** node.

* `char *uci2_get_name(n)`
    * This method return the **name** of node **n**. All node types contain names, and this method can be used freely on all types of nodes.

*  `uci2_q_L(n, i)`
    * This method returns a child of list node **n** specified by index **i**. Node pointer **n** must be a list **[L]** node. If index **i** is out of bound, **NULL** is returned. 

* `int uci2_nc(n)`
    * This method returns child cound for node **n**, it's a helper macro for **n->ch_nr**

* `void uci2_change_value(n, nv)`
    * This method changes the **value** part of node **n** and takes care of freeing the memory used by the old **value** of node **n**.

* `void uci2_change_name(n, nn)`
    * This method changes the **name** part of node **n** and takes care of freeing the memory used by the old **name** of node **n**.

* `uci2_n_t *uci2_add_T(ctx, p, n)`
    * Add **unnamed section** node, also known as type **[T]** node, using `ctx` context, `p` as a parent node, and `n` as the node name in form of NULL terminated string. This method return a pointer to newly created node; the parent node `p` supported by this method is **[@C]** node.
  
* `uci2_n_t *uci2_add_O(ctx, p, n, v)`
    * Add **option** node, also known as **[O]** node, using `ctx` context, `p` as a parent node, and `n` and `v` for the **name = value** structure of options name. Both name and value arguments are NULL terminated strings. This method return a pointer to newly created node; the parent nodes `p` supported by this method is are **[T]** and **[S]** nodes.

* `uci2_n_t *uci2_add_S(ctx, p, n)`
    * Add **names section** node, also known as **[S]** node, using `ctx` context, `p` as a parent node, and `n` as the node name in form of NULL terminated string. This method return a pointer to newly created node; the parent node `p` supported by this method is **[T]** node.
 
* `uci2_n_t *uci2_add_L(ctx, p, n)`
    * Add **list** node, also known as **[L]** node, using `ctx` context, `p` as a parent node, and `n` as the node name in form of NULL terminated string. This method return a pointer to newly created node; the parent nodes `p` supported by this method is are **[T]** and **[S]** nodes.

* `uci2_n_t *uci2_add_I(ctx, p, n)`
    * Add **list item** node, also known as **[I]** node,  using `ctx` context, `p` as a parent node, and `n` as the node name in form of NULL terminated string. This method return a pointer to newly created node; the parent node `p` supported by this method is **[L]** node.

* `void uci2_del(n)`
    * Flag node `n` as deleted. All "deleted" nodes are still present in memory and are freed when parser context is freed by invoking the `uci2_free_ctx(ctx)` method.

* `uci2_iter(parent, item)`
    * This helper macro is used for iterating the child nodes of `parent` and setting the `item` pointer for each consecutive child node. Instead of writing the following: 

* `int uci2_str2bool(str, bool)`
    * Returns boolean value of a given string. The output `bool` argument will be set to `true` if the `str` value matches any of the following strings: `1`, `yes`, `on`, `true`, and `enabled`. Conversely, if the `str` value matches `0`, `no`, `false`, or `disabled`, the output `bool` argument will be set to `false`.
```c
for(int i = 0; i<parent->ch_nr; i++){
    uci2_n_t* item = parent->ch[i];
}
```
You can use the short version by using the following syntax:
```c
uci2_iter(parent, item){
  // print out just for fun
  printf("Node type = [%d]\n", item->nt);
}
```

### Method documentation (also found in **libuci2.h**)
```c
/**
 * Parse configuration file
 *
 * @param[in]       fname   Pointer to configuration 
 *                          filename path
 *
 * @return          Pointer to parser result structure
 *                  or NULL if file does not exist or
 *                  error occurred
 */
uci2_parser_ctx_t *uci2_parse_file(const char *fname);

/**
 * Free parser context
 *
 * @param[in,out]   p       Pointer to parser context
 */
void uci2_free_ctx(uci2_parser_ctx_t *p);

/**
 * Get AST node based on query path (variadic)
 * @param[in]   cfg     Pointer to parser result data
 * @param[in]   ...     Node query string; combined arguments
 *
 * @return      AST node that matches the query path
 */
uci2_ast_t *uci2_get_node_va(uci2_parser_ctx_t *cfg, ...);

/**
 * Get AST node based on query path (variadic va_list)
 * @param[in]   cfg     Pointer to parser result data
 * @param[in]   ...     Node query string; combined arguments
 *
 * @return      AST node that matches the query path
 */
uci2_ast_t *uci2_get_node_va_list(uci2_parser_ctx_t *cfg, va_list ap);

/**
 * Create new parser context
 *
 * @return      Pointer to new parser context
 */
uci2_parser_ctx_t *uci2_new_ctx();

/**
 * Add new AST node to AST tree
 *
 * @param[in]       ctx     Context pointer
 * @param[in,out]   p       Parent node
 * @param[in]       nt      Node type to add
 * @param[in]       n       Node name
 * @param[in]       v       Node value
 *
 * @return          New AST node pointer
 */
uci2_ast_t *uci2_add_node(uci2_parser_ctx_t *ctx, 
			  uci2_ast_t *p, 
			  int nt,
                          char *n, 
                          char *v);

/**
 * Get the option if it exists
 * or create the option if it doesn't exist
 *
 * @param[in]       ctx             Context pointer
 * @param[in]       option_name     Option name
 * @param[in]       ...             Section path
 *
 * @return          Pointer to AST node representing an option
 *                  or NULL if errors occurred
 */
uci2_ast_t *uci2_get_or_create_option(uci2_parser_ctx_t *ctx, 
                                      const char *option_name, 
                                      ...);

/**
 * Export AST to output stream in configuration file
 * format.
 *
 * @param[in]       ctx     Context pointer
 * @param[out]      out     Output stream
 *
 * @return          0 for success or error code
 */
int uci2_export_ctx(uci2_parser_ctx_t *ctx, FILE *out);

/**
 * Export AST to output file specified by filepath. In case of 
 * error, context pointer has to be freed manually 
 *
 * @param[in]       ctx     Context pointer
 * @param[out]      fp      File path to be written to
 *
 * @return          0 for success or error code
 */
int uci2_export_ctx_fsync(uci2_parser_ctx_t *ctx, const char* fp);

/**
 * @brief Helper macro for uci2_get_node_va
 *
 * Query is prefixed with "@C" to get configuration data;
 * some other type of root nodes may be added for future
 * use
 *
 * @param[in]       cfg     Root AST node
 */
uci2_ast_t *uci2_q(ctx, ...);

/**
 * Get listem item (I) node specified by indes (i)
 *
 * @param[in]       n       List node (L)
 * @param[in]       i       Index of node's item (I)
 *
 * @return          Pointer to list item or NULL if index is out 
 *                  of bounds or 'n' type is not UCI2_NT_LIST
 */
uci2_ast_t *uci2_q_L(n, i);

/**
 * Get child count for node
 *
 * @param[in]       n       Node pointer
 * 
 * @return          Child count for node 'n'
 */
int uci2_nc(n);

/**
 * Print entire AST tree to stdout
 *
 * @param[in]       ctx     Context pointer
 */
void uci2_ctx_print(ctx);

/**
 * Print node and its children to stdout
 *
 * @param[in]       n      Node pointer
 */
void uci2_print(n);

/**
 * Get value of AST node
 *
 * @param[in]       n       Node pointer
 *
 * @return          String pointer of node's value
*/
char *uci2_get_value(n);
/**
 * Get name of AST node
 *
 * @param[in]       n       Node pointer
 *
 * @return          String pointer of node's name
*/
char *uci2_get_name(n);

/**
 * Add AST node (short for uci2_add_node)
 *
 * @param[in]       ctx     Parser context pointer
 * @param[in]       p       Pointer to parent node
 * @param[in]       nt      Type of new node
 * @param[in]       n       Node name string
 * @param[in]       v       Node value string
 *
 * @return          Pointer to newly created node
*/
uci2_ast_t *uci2_add(ctx, p, nt, n, v);

/**
 * Get Configuration root node
 *
 * @param[in]       ctx     Parser context pointer
 *
 * @return          Pointer to root configuration node (@C)
 */
uci2_ast_t *UCI2_CFG_ROOT(ctx);

/**
 * Add Type (un-named) AST node (T)
 *
 * @param[in]       ctx     Parser context pointer
 * @param[in]       p       Parent node pointer
 * @param[in]       n       String used for node's name
 *
 * @return          Pointer to newly created node
 */
uci2_ast_t *uci2_add_T(ctx, p, n);

/**
 * Add Options AST node (O)
 * 
 * @param[in]       ctx     Parser context pointer
 * @param[in]       p       Parent node pointer
 * @param[in]       n       String used for node's name
 * @param[in]       v       String used for node's value
 *
 * @return          Pointer to newly created node
 */
uci2_ast_t *uci2_add_O(ctx, p, n, v);

/**
 * Add Options AST node (O)
 *
 * @param[in]       ctx     Parser context pointer
 * @param[in]       o       Option name
 * @param[in]       ...     Parent path
 *
 * @return          Pointer to newly created node
 */
uci2_ast_t *uci2_get_or_create_O(ctx, o, ...)

/**
 * Add Section AST node (S)
 *
 * @param[in]       ctx     Parser context pointer
 * @param[in]       p       Parent node pointer
 * @param[in]       n       String used for node's name
 *
 * @return          Pointer to newly created node
 */
uci2_ast_t *uci2_add_S(ctx, p, n);

/**
 * Add List AST node (L)
 *
 * @param[in]       ctx     Parser context pointer
 * @param[in]       p       Parent node pointer
 * @param[in]       n       String used for node's name
 *
 * @return          Pointer to newly created node
 */
uci2_ast_t *uci2_add_L(ctx, p, n);

/**
 * Add List Item AST node (I)
 *
 * @param[in]       ctx     Parser context pointer
 * @param[in]       p       Parent node pointer
 * @param[in]       n       String used for node's name
 *
 * @return          Pointer to newly created node
 */
uci2_ast_t *uci2_add_I(ctx, p, n);

/**
 * Iterate child nodes of 'n', access each child with 'c' pointer
 *
 * @param[in]       n       Pointer to parent node whose child nodes
 *                          should be iterated
 * @param[in]       c       Name of pointer used to point to each child
 *                          of parent node; this pointer is updated in
 *                          each iteration
 */
void uci2_iter(n, c);

/**
 * Change node 'n' value part to 'nv' string
 *
 * @param[in]       n       Pointer to node whose value member
 *                          will be changed
 * @param[in]       nv      String to replace the old value
 */
void uci2_change_value(n, nv);

/**
 * Change node 'n' name part to 'nn' string
 *
 * @param[in]       n       Pointer to node whose value member
 *                          will be changed
 * @param[in]       nn      String to replace the old name
*/
void uci2_change_name(n, nn);

/**
 * Mark node as deleted
 *
 * @param[in]       n       Pointer to node which will be
 *                          flaged as deleted
 */
void uci2_del(n);
```
