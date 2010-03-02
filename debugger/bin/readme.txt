このプログラムはQt4のLGPL版を使用しています。
http://qt.nokia.com/title-jp?set_language=ja&cl=ja
リバースエンジニアリングを許可します。


/////////////////////////
使用方法

debugger.exeを立ち上げます。
このデバッガはTCPでクライアントとデバッグ情報をやり取りしますので
ネットワーク使用の警告が出る場合は許可してください。


・メニューのFile -> New Projectで新しいプロジェクトを作成します。

・左のProjectというツリービューにエクスプローラーからファイルをドロップアンドドラッグして追加します。

・Projectのツリービューのファイル名をダブルクリックすると、そのファイルを真ん中のコードエディタ部分で参照できます。
　簡単な編集もできますが、色づけがされる以外はメモ帳レベルです。
　そのコードエディタの行番号の左の部分をクリックすると、ブレークポイントを設置できます。

・右のExprツリービューで評価したい式を記述できます。

・右のCallStackツリービューで関数呼び出し履歴が参照できます。


/////////////////////////
クライアント側、つまりデバッグしたいプログラム側の準備

#include "../src/xtal/xtal.h"
#include "../src/xtal/xtal_macro.h"

#include "../src/xtal/xtal_lib/xtal_winthread.h"
#include "../src/xtal/xtal_lib/xtal_cstdiostream.h"
#include "../src/xtal/xtal_lib/xtal_winfilesystem.h"
#include "../src/xtal/xtal_lib/xtal_chcode.h"
#include "../src/xtal/xtal_lib/xtal_errormessage.h"
#include "../src/xtal/xtal_lib/xtal_debugger.h" // デバッガライブラリ

int main(int argc, char** argv){
	CStdioStdStreamLib cstd_std_stream_lib;
	WinThreadLib win_thread_lib;
	WinFilesystemLib win_filesystem_lib;
	SJISChCodeLib sjis_ch_code_lib;

	Setting setting;
	setting.thread_lib = &win_thread_lib;
	setting.std_stream_lib = &cstd_std_stream_lib;
	setting.filesystem_lib = &win_filesystem_lib;
	setting.ch_code_lib = &sjis_ch_code_lib;

	initialize(setting);

	{
		Debugger debugger; // デバッガとやりとりするクラス
		debugger.attach(xnew<DebugStream>("127.0.0.1", "13245")); // 接続する

		require("test"); // デバッガにtext.xtalが登録されているならば、それをコンパイルして実行する
		// 登録されていなければ、loadと同じ挙動をする

		// 後は定期的にdebugger.update();を呼び出すこと
		// ゲームであれば毎フレーム呼び出すこと
	}
		
	uninitialize();

	return 0
}