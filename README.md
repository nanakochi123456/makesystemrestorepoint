makesystemrestorepoint
Copyright 2014 かわいいななこ
========

かなり単純に作られた、システムの復元ポイント生成プログラムです。

■環境
Visual Studio 2013 ほぼC言語

■使い方
makesystemrestorepoint.exe [システムの復元に入れるメッセージ|/?]

メッセージを入れないと、「Demonstration Restore Point」が入ります。

40文字ぐらいを超えると、エラーになります。

■バグじゃないの
Windows 8.1ですと、システムの復元ポイントが作成されない場合があるみたいなの
たいていは、全削除すれば、作成されるなの

Windows 8.1ですと、システムの復元ポイントはバックグラウンドで作成されるみたいなの

■仕様です
runasしないなの

■ぼやき
Windows 8.1だと、なかなかシステムの復元ポイントを生成してくれません。
本当に何か変更しないと、やってくれないようです。
かつ、瞬間で終わるように見えるのですが、実際はバックグラウンドで
実行されており、後から反映されます。