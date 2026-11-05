/* Tomato -> Aidoku legacy ABI for Rakuyomi 1.22.2.
 * Experimental; requires the adapted base64-page backend.
 * Based on the local MANGA Plus legacy adapter.
 */
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef struct { const u8 *p; int n; } Slice;
typedef struct { Slice rest, data; u64 number; int tag, wire; } Reader;
#define IMPORT(m,n) __attribute__((import_module(m),import_name(n)))
#define EXPORT(n) __attribute__((export_name(n)))
#define STR(s) (s), (int)(sizeof(s)-1)
IMPORT("net","init") int net_init(int);
IMPORT("net","close") void net_close(int);
IMPORT("net","set_url") void net_url(int,const char*,int);
IMPORT("net","set_header") void net_header(int,const char*,int,const char*,int);
IMPORT("net","send") void net_send(int);
IMPORT("net","get_data_size") int net_size(int);
IMPORT("net","get_data") void net_data(int,u8*,int);
IMPORT("net","get_status_code") int net_status(int);
IMPORT("std","read_int") long long std_int(int);
IMPORT("std","create_array") int array_new(void);
IMPORT("std","array_append") void array_add(int,int);
IMPORT("std","array_len") int array_len(int);
IMPORT("std","array_get") int array_get(int,int);
IMPORT("std","object_get") int object_get(int,const char*,int);
IMPORT("std","string_len") int string_len(int);
IMPORT("std","read_string") void string_read(int,char*,int);
IMPORT("std","destroy") void destroy(int);
IMPORT("env","print") void log_text(const char*,int);
IMPORT("aidoku","create_manga_result") int manga_result(int,int);
IMPORT("aidoku","create_manga") int manga_new(
 const char*,int,const char*,int,const char*,int,const char*,int,
 const char*,int,const char*,int,const char*,int,int,int,int,int,int,int);
IMPORT("aidoku","create_chapter") int chapter_new(
 const char*,int,const char*,int,float,float,double,
 const char*,int,const char*,int,const char*,int);
IMPORT("aidoku","create_page") int page_new(int,const char*,int,const char*,int,const char*,int);


IMPORT("net","json") int net_json(int);
IMPORT("defaults","get") int defaults_get(const char*,int);
#define API "https://prod-api.tomatoanimes.com"
static int len(const char*s){int n=0;while(s[n])n++;return n;}
static void fail(const char*s){log_text(s,len(s));__builtin_trap();}
static char*cat(char*p,const char*s){while(*s)*p++=*s++;*p=0;return p;}
static void num(char*p,u64 n){char t[24];int k=0;do{t[k++]=48+n%10;n/=10;}while(n);while(k)*p++=t[--k];*p=0;}
static void text(int d,char*p,int cap){int n=string_len(d);if(n<0||n>=cap)fail("Texto invalido");string_read(d,p,n);p[n]=0;}
static void prop(int d,const char*k,char*p,int cap){int v=object_get(d,k,len(k));text(v,p,cap);destroy(v);}
static u64 integer(int d,const char*k){int v=object_get(d,k,len(k));u64 n=std_int(v);destroy(v);return n;}
static int api(const char*path){char url[1024],auth[8192];cat(cat(url,API),path);int d=defaults_get(STR("token"));cat(auth,"Bearer ");text(d,auth+7,sizeof(auth)-7);destroy(d);if(!auth[7])fail("Configure a sessao Tomato");int r=net_init(0);net_url(r,url,len(url));net_header(r,STR("Authorization"),auth,len(auth));net_header(r,STR("User-Agent"),STR("tomato-android"));net_send(r);d=net_status(r);int status=std_int(d);destroy(d);if(status!=200)fail("Sessao Tomato expirada ou falha de rede");int j=net_json(r);net_close(r);return j;}
static int title(int d){char id[24],name[1024],cover[2048];num(id,integer(d,"id"));prop(d,"name",name,sizeof(name));prop(d,"thumbnail",cover,sizeof(cover));return manga_new(id,len(id),cover,len(cover),name,len(name),0,0,0,0,0,0,0,0,0,0,0,1,0,1);}
EXPORT("get_manga_list") int get_manga_list(int filters,int page){(void)filters;int out=array_new();if(page>1)return manga_result(out,0);int root=api("/v2/manga/feed"),groups=object_get(root,STR("data"));int seen[2048],count=0;for(int i=0;i<array_len(groups);i++){int g=array_get(groups,i),items=object_get(g,STR("data"));for(int k=0;k<array_len(items);k++){int item=array_get(items,k),id=integer(item,"id"),dup=0;for(int j=0;j<count;j++)if(seen[j]==id)dup=1;if(!dup){if(count==2048)fail("Catalogo muito grande");seen[count++]=id;int m=title(item);array_add(out,m);destroy(m);}destroy(item);}destroy(items);destroy(g);}destroy(groups);destroy(root);return manga_result(out,0);}
static void path_id(int d,const char*base,char*p){char id[32];prop(d,"id",id,sizeof(id));for(int i=0;id[i];i++)if(id[i]<48||id[i]>57)fail("ID invalido");cat(cat(p,base),id);}
EXPORT("get_chapter_list") int get_chapter_list(int manga){char path[128];path_id(manga,"/manga/chapters/query/",path);int root=api(path),items=object_get(root,STR("data")),out=array_new();for(int i=0;i<array_len(items);i++){int d=array_get(items,i);char id[32],name[1024];num(id,integer(d,"id"));prop(d,"name",name,sizeof(name));int c=chapter_new(id,len(id),name,len(name),0,(float)integer(d,"number"),0,0,0,0,0,STR("pt-BR"));array_add(out,c);destroy(c);destroy(d);}destroy(items);destroy(root);return out;}
#define IMG_CAP (8*1024*1024)
static u8 image[IMG_CAP];
static char encoded[(IMG_CAP+2)/3*4+1];
static int base64(const u8*data,int n) {
 const char*alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";int k=0;
 for(int i=0;i<n;i+=3) {
  u32 v=(u32)data[i]<<16;if(i+1<n)v|=(u32)data[i+1]<<8;if(i+2<n)v|=data[i+2];
  encoded[k++]=alphabet[v>>18];encoded[k++]=alphabet[(v>>12)&63];encoded[k++]=i+1<n?alphabet[(v>>6)&63]:'=';encoded[k++]=i+2<n?alphabet[v&63]:'=';
 }encoded[k]=0;return k;
}


EXPORT("get_page_list") int get_page_list(int chapter){char path[128];path_id(chapter,"/manga/pages/query/",path);int root=api(path),items=object_get(root,STR("data")),out=array_new();for(int i=0;i<array_len(items);i++){int d=array_get(items,i);char url[4096];prop(d,"page_url",url,sizeof(url));const char*prefix="https://cdn.mangapipe.com/";for(int k=0;prefix[k];k++)if(url[k]!=prefix[k])fail("Dominio de imagem desconhecido");int r=net_init(0);net_url(r,url,len(url));net_header(r,STR("User-Agent"),STR("tomato-android"));net_send(r);int st=net_status(r),status=std_int(st);destroy(st);int n=net_size(r);if(status!=200||n<=0||n>IMG_CAP)fail("Falha ao baixar imagem Tomato");net_data(r,image,n);net_close(r);int size=base64(image,n);int pg=page_new((int)integer(d,"page_number"),0,0,encoded,size,0,0);array_add(out,pg);destroy(pg);destroy(d);}destroy(items);destroy(root);return out;}
