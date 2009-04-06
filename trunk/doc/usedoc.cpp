
/** \page usepage C++組み込み
- \subpage useinitialize
- \subpage useexcept
- \subpage usehelloworld
- \subpage usestring
- \subpage useanyptr
- \subpage usefun
- \subpage usefuncall
- \subpage usesend
- \subpage usenew
- \subpage userefmember
- \subpage usedefmember
- \subpage useat
- \subpage usereturn
- \subpage useimport
- \subpage usecppclass
- \subpage usebytecode
*/

/** \page useinitialize Xtalの初期化
例として、Windowsでの初期化方法を説明します。\n
\code
#include <xtal.h>
#include <xtal_macro.h> // Xidなど便利なマクロが定義されている

#include <xtal_lib/xtal_cstdiostream.h> // CStdioStdStreamLibのため
#include <xtal_lib/xtal_winthread.h> // WinThreadLibのため
#include <xtal_lib/xtal_winfilesystem.h> // WinFilesystemLibのため
#include <xtal_lib/xtal_chcode.h> // SJISChCodeLibのため
#include <xtal_lib/xtal_errormessage.h> // bind_error_message()のため

int main(int argc, char** argv){
	using namespace xtal;

	CStdioStdStreamLib cstd_std_stream_lib; // stdin, stdout, stderrはCの標準ライブラリを使う
	WinThreadLib win_thread_lib; // Windowsのスレッドを使う
	WinFilesystemLib win_filesystem_lib; // Windowsのファイルシステムを使う 
	SJISChCodeLib sjis_ch_code_lib; // SJISを使う

	// 環境依存である機能についてどれを使うかを設定
	Setting setting; 
	setting.thread_lib = &win_thread_lib;
	setting.std_stream_lib = &cstd_std_stream_lib;
	setting.filesystem_lib = &win_filesystem_lib;
	setting.ch_code_lib = &sjis_ch_code_lib;
	
	// ここで指定したthread_libなどのポインタが示す先のオブジェクトは、
	// uninitializeを呼び出すまで存在している必要があります。

	// Xtalを初期化
	initialize(setting);

	// エラーメッセージをバインド
	bind_error_message();

	// ここでスクリプトの実行を行う

	// Xtalを破棄
	uninitialize();

	return 0;
}
\endcode

Xtalは環境依存である機能は、簡単に取替えができるようになっています。\n
*/

/** \page usehelloworld ファイルに書かれているXtalを実行
次のようなHello, Worldを出力するスクリプト、HelloWorld.xtalがあるとします。\n
\code
// HelloWorld.xtal
println("Hello, World");
\endcode
これをC++から呼び出すにはload関数を使います。\n
\code
// C++
load("HelloWirld.xtal");

// コンパイルエラーや実行時エラーがあったら捕捉する。
XTAL_CATCH_EXCEPT(e){
	stderr_stream()->println(e); // stderrに例外オブジェクトを出力する
}

\endcode
load関数は呼ばれるたびにHelloWorldをコンパイルして実行します。\n   
*/

/** \page useexcept Xtalで発生した例外を捕捉する
XTAL_CATCH_EXCEPT(e)を使います。\n
\code
any->call();

XTAL_CATCH_EXCEPT(e){
	// call内で例外がセットされたらここを通る
	// eに例外オブジェクトがセットされる。
}
else{
	// 例外が発生していないならここを通る
}
\endcode

*/

/** \page usestring 文字列のXtalソースを実行
ファイルとしてではなく、文字列としてソースを保持していてそれを実行したい場合は、compile関数を使い、結果を呼び出します。 \n
\code
const char* source = " println(\"Hello, World\"); ";

// コンパイルエラーがあれば、codeはnullが返る
if(CodePtr code = compile(source)){
	code->call();
}

XTAL_CATCH_EXCEPT(e){
	stderr_stream()->println(e); // stderrに例外オブジェクトを出力する
}
\endcode
*/

/** \page useanyptr AnyPtr
AnyPtrはあらゆるXtalのオブジェクトを保持できる、スマートポインタ型です。 \n   
AnyPtrは、正確にはSmartPtr<Any>型のtypedefです。 \n

\section int2anyptr 整数や浮動小数点数型とAnyPtrを相互変換する 
AnyPtrは全ての整数や浮動小数点数を受け取るコンストラクタが定義されているので、C++の整数からAnyPtrへの変換は自動的に行われます。\n
\code
AnyPtr any = 100;
AnyPtr value = 10.0f;
\endcode
逆にAnyPtr型に入れたものを整数に変換したい場合、Any::to_iメンバ関数やAny::to_fメンバ関数を使います。\n
\code
int i = any->to_i();
float f = value->to_f();
\endcode

\section str2anyptr 文字列とAnyPtrを相互変換する 
AnyPtrはconst char*を受け取るコンストラクタが定義されているため、文字列リテラル等からAnyPtrへの変換は自動的に行われます。\n
\code
AnyPtr str = "string";
\endcode
逆にAnyPtrからC++の文字列に直すには、Any::to_sメンバ関数を使います。\n
to_sはStringPtrを返してくるので、StringPtrで受け取ります。\n
StringPtrはSmartPtr<String>のtypedefです。\n    
\code
StringPtr s = str->to_s();
\endcode
そして、String::c_str関数で、const char*として受け取れます。\n
\code
const char* ccp = s->c_str();
\endcode

\section anyptr2any AnyPtr型を他の型に変換する
例えば、Stringを保持しているAnyPtrをStringPtr型に変換する場合、AnyPtr::to_s()を使いますが、その他にもptr_cast関数を使うことでString型に変換できます。\n
ptr_cast関数は次のような関数です。    \n

\code
template<class T>
SmartPtr<T> ptr_cast(const AnyPtr& from);
\endcode
dynamic_cast等のC++のキャストと同じ形式で呼び出せます。    \n
\code
AnyPtr astr = "string";
StringPtr str = ptr_cast<String>(astr);
\endcode
もしキャストに失敗したら、nullを返します。\n  
\n
絶対にこれはStringPtrだ、と確定している場合、unchecked_ptr_castが使えます。\n
ptr_castは継承関係を調べるため若干時間がかかりますが、unchecked_ptr_castは高速です。\n
\code
AnyPtr astr = "string";
StringPtr str = unchecked_ptr_cast<String>(astr);
\endcode
*/

/** \page usefun C++の関数をXtalから呼べるように変換する
C++の関数をXtalの関数から呼べる形式に変換するには、fun関数を使います。\n 
fun関数を簡単に言うと、C++の関数ポインタを渡せば、Xtalで扱えるオブジェクトを作って返す、という関数です。\n
\n
fun関数は次のような形式です。 \n
\code
NativeFunPtr fun(R (*pointer_to_function)());
NativeFunPtr fun(R (*pointer_to_function)(A0));
NativeFunPtr fun(R (*pointer_to_function)(A0, A1));
NativeFunPtr fun(R (*pointer_to_function)(A0, A1, A2));
NativeFunPtr fun(R (*pointer_to_function)(A0, A1, A2, A3));
NativeFunPtr fun(R (*pointer_to_function)(A0, A1, A2, A3, A4));
\endcode
RやA0は、関数の戻り値、引数が自動判定されて決まります。\n
\code
void foo(int a, int b){
    printf("%d\n", a+b);
}

void bar(){
    printf("%s\n", "bar");
}

void test(){
	NativeFunPtr f = fun(&foo);
	NativeFunPtr b = fun(&bar);
}
\endcode

名前付き引数とそのデフォルト値を指定したい場合、paramメンバ関数を使います。    
\code
void foo(int key1, int key2){
    printf("%d\n", key1+key2);
}

void test(){
	// 一つ目の引数をkey1と名づけ、デフォルト値を50に
	// 二つ目の引数をkey2と名づけ、デフォルト値を100にする
	NativeFunPtr f = fun(&foo)->param(1, Xid(key1), 50)->param(2, Xid(key2), 100);
}
\endcode
*/

/** \page usefuncall 関数の呼び出し
AnyPtrに格納されているのがC++の関数かXtalの関数かに関わらず、Any::callを使います。\n
\code
void add(int a, int b){
    printf("%d\n", a+b);
}
void test(){
	AnyPtr add = fun(&add);
	AnyPtr ret = add->call(100, 200);
}
\endcode
とAnyPtr型の変数に()を付けて引数を渡すというとても直感的な関数の呼び出しができます。\n
名前つき引数の呼び出しも下のように手軽に記述できます。\n
\code
void add(int key1, int key2){
     printf("%d\n", key1+key2);
}
void test(){
	AnyPtr add = fun(&add)->param(1, Xid(key1), 10)->param(2, Xid(key2), 20);
	AnyPtr ret = add->call(Named(Xid(key2), 50)); // => 60
}
\endcode
*/

/** \page usesend メソッド呼び出し
AnyPtrに格納されているのがC++のオブジェクトかXtalのオブジェクトかに関わらず、Any::sendを使います。\n
\code
int len = obj->send(Xid(length))->to_i();
AnyPtr ret = obj->send(Xid(slice), 0, 2);
printf("%s\n", ret->to_s()->c_str());
\endcode
*/

/** \page usenew C++のオブジェクトをnewする
xnew<T>テンプレート関数を使うと、C++のオブジェクトを、Xtalでも扱えるオブジェクトとして作成できます。\n  
xnew<T>関数は、SmartPtr<T>型を返します。\n
\n
例えば、組み込みの配列のArrayクラスはC++で記述されていますが、これをC++で生成するには次のように書きます。\n
\code
// ArrayPtr は SmartPtr<Array> のtypedef
ArrayPtr ary = xnew<Array>(10); //長さ10の配列を生成
ary->push_back(10);
\endcode
 
ユーザーが定義したクラスもxnewで生成出来ます。\n
\code
class Foo{
pubic:
    void bar(){}
};
SmartPtr<Foo> foo = xnew<Foo>();
foo->bar();
\endcode
これらのオブジェクトは、すべてのスマートポインタが参照しなくなれば、自動的にガーベージコレクションで削除されるので、明示的なdeleteは必要ありません\n
*/

/** \page userefmember メンバの取得
Xtalでは、クラス、libオブジェクトなどのメンバを取得するのに、「Int::foo」と書きますが、それをC++で書く方法です。\n
\n
これにはAny::memberメンバ関数を使います。\n
取得したいメンバの名前を第一引数として渡します。\n    
もし取得したいメンバが無い場合、nullが返ります。\n  
\code 
AnyPtr foo = cls->member(Xid(foo));
AnyPtr bar = lib()->member(Xid(bar)):
\endcode
*/

/** \page usedefmember メンバの定義
C++からクラスやモジュール、libオブジェクトのメンバを追加定義する方法です。「Int::foo : 100;」相当です。

これにはAny::defメンバ関数を使います。    
第一引数に定義名、第二引数に定義する値を入れます。   

 
cls->def("foo", 100);
lib()->def("bar", fun(&add));
*/


/** \page useat 配列、連想配列の要素取得、設定
Any::atを使って、要素の取得を行います。\n
\code
ret = ary->at(0);
ret = map->at("key");
\endcode
 
値の設定は、Any::set_atを使います。\n
\code
any->set_at(0, 10);
map->set_at("test", 5);
\endcode
*/

/** \page usereturn Xtalからオブジェクトを受け取る
Xtalから値をC++に返すにはトップレベルでreturn文を使います。\n
\code
// test.xtal
return 100 + 20;
\endcode
 
\code
// C++
// test.xtalファイルを実行し、returnされたオブジェクトを得る
AnyPtr ret = load("test.xtal");

// 例外が発生していないかチェック
XTAL_CATCH_EXCEPT(e){
	stderr_stream()->println(e);
}
else{
	int value = ret->to_i();
	printf("%d\n", value);
}
\endcode
*/

/** \page useimport Xtalへオブジェクトを渡す
ライブラリを管理するlibオブジェクトのメンバとして定義してあげる方法があります。\n
\code
// C++
void cpp_fun(){
    puts("cpp_fun");
}

void test(){
	// libオブジェクトに登録
	lib()->def(Xid(cpp_value), fun(&cpp_fun));
	load("test.xtal");
}
\endcode

\code
// test.xtal
lib::cpp_value(); // => cpp_fun
\endcode
メンバは基本的に再定義不可能なので、呼び出しのたびに設定するオブジェクトを変えたいなどといった場合に、この方法では対応できません。\n
\n
他に実行時に引数として渡す方法があります。\n

\code
// C++
void test(){
	// コンパイルして、それをcallで実行する。
	if(CodePtr code = compile("test.xtal")){
		code->call(100, 200);
	}
}
\endcode

\code
// test.xtal
args: ...; // トップレベルの...はスクリプト実行する際に渡された引数が入っている。
args[0].p; //=> 100
args[1].p; //=> 200
\endcode

Code::filelocalでそのコードのfilelocalを取り出して、変数を定義することもできます。\n
\code
// C++
void test(){
	// コンパイルして、それをcallで実行する。
	if(CodePtr code = compile("test.xtal")){
		code->def(Xid(foo), 100);
		code->call();
	}
}
\endcode

\code
// test.xtal
foo.p; //=> 100
\endcode
*/

/** \page usecppclass C++のクラスをバインドする
\section secvec2d 2次元のベクトルクラスをXtalで使えるようにする例
\code
// test.cpp
class Vector2D{
	float x, y;
	
	Vector2D(float x = 0, float y = 0)
		:x(x), y(y){}
	
	float length() const{
		return sqrt(x*x + y*y);
	}
	
	void normalize(){
		float len = length();
		x /= len;
		y /= len;
	}
};

// XTAL_PREBINDの中で継承関係の登録、コンストラクタの登録を行う
XTAL_PREBIND(Vector2D){
	// itはClassPtrである。
	// it->でClassクラスのメンバ関数が呼べる
	
	// コンストラクタの登録
	it->def_ctor2<Vector2D, float, float>()->param(1, Xid(x), 0)->param(2, Xid(y), 0);
}

// XTAL_BINDの中でメンバ関数の登録を行う
XTAL_BIND(Vector2D){
	// itはClassPtrである。
	// it->でClassクラスのメンバ関数が呼べる

	it->def_var(Xid(x), &Vector2D::x); // メンバ変数xのセッタ、ゲッタを登録
	it->def_var(Xid(y), &Vector2D::y); // メンバ変数yのセッタ、ゲッタを登録
	it->def_method(Xid(length), &Vector2D::length); // メンバ関数lengthを登録
	it->def_method(Xid(normalize), &Vector2D::normalize); // メンバ関数lengthを登録
}

void test(){
	// libオブジェクトに登録
	lib()->def(Xid(Vector), cpp_class<Vector2D>());
}
\endcode

\code
// test.xtal
vec: lib::Vector2D(10, 20);
vec.length.p;
vec.normalize;
vec.length.p;
\endcode

\section secfoo 継承関係があるクラスをバインドする
C++のクラスに継承関係がある場合、Class::inheritを使って、継承関係を登録する必要があります。\n
これをしなければ、キャストに失敗します。\n
\code
class Foo{
public:
    int a;
    Foo(int value = 0){ a = value; }
    int add(int b){ return a+=b; }
};

class SubFoo : public Foo{
public:
    SubFoo(){}
};

XTAL_PREBIND(Foo){
	it->def_ctor1<Foo>()->param(1, Xid(value), 0);
}

XTAL_BIND(Foo){
	it->def_method(Xid(add), &Foo::add);
}

XTAL_PREBIND(SubFoo){
	// inheritで継承関係を結ぶ
	// cpp_class<Foo>()でFooクラスのクラスオブジェクトを取得できる
	it->inherit(cpp_class<Foo>());
	it->def_ctor0<SubFoo>();
}

void test(){
    // libオブジェクトに登録する
    lib()->def(Xid(Foo), cpp_class<Foo>());
    lib()->def(Xid(SubFoo), cpp_class<SubFoo>());

    // 実行して結果を受け取る
	AnyPtr ret = load("test.xtal");
    
    // ここでretにはSubFooクラスのインスタンスが入っている
    
    // ptr_castでSubFooとして取り出せる
    SmartPtr<SubFoo> subfooref = ptr_cast<SubFoo>(ret);
    
    // SubFooと継承関係を結んだFooとしても取り出せる
    SmartPtr<Foo> fooref = ptr_cast<Foo>(ret);
}
\endcode

\code
// test.xtal
// libオブジェクトに登録された、SubFooクラスを取り出してインスタンスを生成する。
subfoo: lib::SubFoo(); 
return subfoo; // subfooをreturnする
\endcode
*/

/** \page usebytecode バイトコードにコンパイルする
\code
// ファイルをコンパイルする
if(CodePtr code = compile_file("test.xtal")){
	// test.xtalcにバイトコードを保存する
	FileStreamPtr fs = xnew<FileStream>("test.xtalc", "w");
	fs->serialize(code);
}
\endcode

\code
// バイトコードを読み出す
FileStreamPtr fs = xnew<FileStream>("test.xtalc", "r");
if(CodePtr code = ptr_cast<Code>(fs->deserialize())){
	// 実行する
	code->call();
}
\endcode
*/

/** \page
関数が沢山ある場合は、それらをClassとして纏め、それを渡すとスマートです。    
Classクラスも最基底ではAnyPtrを継承しています。そのため、AnyPtrに自動変換可能です。    
    
Classクラスは関数の登録に便利なdef_funメンバ関数を持っています。    
Class::def_funは any->def("foo", fun(&foo)) のショートカットです。    

include <xtal.h>
using namespace xtal;

int add(int a, int b){ return a+b; }
int sub(int a, int b){ return a-b; }
int mul(int a, int b){ return a*b; }
int div(int a, int b){ return a/b; }

void test(){
    // Xtalの初期化
    initialize(); 
    
    try{

        ClassPtr ops = xnew<Class>("ops"); // opsクラスを作る
        ops->def_fun("add", &add); // addという名前のメンバとしてadd関数を登録
        // ops->def("add", fun(&add)); // これと上は同じ
    
        ops->def_fun("sub", &sub);
        ops->def_fun("mul", &mul);
        
        // paramsで名前付き引数を付けられる
        ops->fun("div", &div)->params("a", 0, "b", 1); 
    
        // libオブジェクトに登録する
        lib()->def("ops", ops);
    
        // test.xtalをロードし、実行する
        load("test.xtal");
        
    }catch(AnyPtr e){
         fprintf(stderr, "%s\n", e->to_s()->c_str());
    }
}
*/