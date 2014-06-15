#pragma once

// cevio.h
// 6/15/2014 jichi
// See: http://guide2.project-cevio.com/interface
// - COM: http://guide2.project-cevio.com/interface/com
// - .NET: http://guide2.project-cevio.com/interface/dotnet

#include <objbase.h>

// 外部ソフトにトーク機能を提供します。
interface ITalker
{

  uint Volume { get; set; }
  // 音の大きさ（0～100）を取得または設定します。

  uint Speed { get; set; }
  // 話す速さ（0～100）を取得または設定します。

  uint Tone { get; set; }
  // 音の高さ（0～100）を取得または設定します。

  uint Alpha { get; set; }
  // 声質（0～100）を取得または設定します。

  ITalkerComponentArray Components { get; }
  // 現在のキャストの感情パラメータマップを取得します。
  // 備考：
  //   内容はCastによって変化します。
  //   ※製品版【CeVIO Creative Studio】に含まれるキャストの場合、以下の感情があります。
  //   　『さとうささら』 → "普通", "元気", "怒り", "哀しみ"
  //  『すずきつづみ』 → "クール", "照れ"
  //  『タカハシ』 → "普通", "元気", "へこみ"
  // 注意点：
  //   型は、Visual C++環境でスマートポインタを利用する場合、下記に置き換えられます。
  //   ITalkerComponentArrayPtr

  string Cast { get; set; }
  // キャストを取得または設定します。

  IStringArray AvailableCasts { get; }
  // 利用可能なキャスト名を取得します。
  // 備考：
  //   キャストの取り揃えは、インストールされている音源によります。
  // 注意点：
  //   型は、Visual C++環境でスマートポインタを利用する場合、下記に置き換えられます。
  //   IStringArrayPtr

  ISpeakingState Speak(string text);
  // 指定したセリフの再生を開始します。
  // 引数：
  //   text - セリフ。
  // 戻り値：
  //   再生状態を表すオブジェクト。
  // 備考：
  //   再生終了を待たずに処理が戻ります。
  //   再生終了を待つには戻り値（ISpeakingState）のWaitを呼び出します。
  // 注意点：
  //   型は、Visual C++環境でスマートポインタを利用する場合、下記に置き換えられます。
  //   ISpeakingStatePtr

  bool Stop();
  // 再生を停止します。
  // 戻り値：
  //   成功した場合はtrue。それ以外の場合はfalse。

  double GetTextDuration(string text);
  // 指定したセリフの長さを取得します。
  // 引数：
  //   text - セリフ。
  // 戻り値：
  //   長さ。単位は秒。


  IPhonemeDataArray GetPhonemes(string text);
  // 指定したセリフの音素単位のデータを取得します。
  // 引数：
  //   text - セリフ。
  // 戻り値：
  //   音素単位のデータ。
  // 備考：
  //   リップシンク等に利用できます。
  // 注意点：
  //   型は、Visual C++環境でスマートポインタを利用する場合、下記に置き換えられます。
  //   IPhonemeDataArrayPtr

  bool OutputWaveToFile(string text, string path);
  // 指定したセリフをWAVファイルとして出力します。
  // 引数：
  //   text - セリフ。
  //   path - 出力先パス。
  // 戻り値：
  //   成功した場合はtrue。それ以外の場合はfalse。
  // 備考：
  //   出力形式はサンプリングレート48kHz, ビットレート16bit, モノラルです。

}

// キャストの感情パラメータマップを表すオブジェクト。
interface ITalkerComponentArray
{

  int Length { get; }
  // 要素数を取得します。

  ITalkerComponent At(int index) { get; }
  // 指定したインデックスの要素を取得します。
  // 引数：
  //   index - インデックス。
  // 戻り値：
  //   要素。

  ITalkerComponent ByName(string name) { get; }
  // 指定した名前の要素を取得します。
  // 引数：
  //   name - 名前。
  // 戻り値：
  //   要素。

  ITalkerComponentArray Duplicate();
  // 配列を複製します。
  // 戻り値：
  //   複製した配列のインスタンス。

}

// 感情パラメータの単位オブジェクト。
interface ITalkerComponent
{

  string Id { get; }
  // 識別子を取得します。

  string Name { get; }
  // 感情の名前を取得します。（文字コードはUnicode）

  uint Value { get; set; }
  // 感情の値（0～100）を取得または設定します。

}

// 再生状態を表すオブジェクト。
interface ISpeakingState
{

  bool IsCompleted { get; }
  // 再生が完了したかどうかを取得します。
  // 完了した場合はtrue。（失敗を含む）それ以外の場合はfalse。

  bool IsSucceeded { get; }
  // 再生が成功したかどうかを取得します。
  // 成功した場合はtrue。それ以外の場合はfalse。

  void Wait();
  // 再生終了を待ちます。

  void Wait_2(double timeout);
  // 再生終了を待ちます。
  // 引数：
  //   timeout - 最大待機時間。単位は秒。（0未満は無制限）

}

// 音素データの配列を表すオブジェクト。
interface IPhonemeDataArray
{

  int Length { get; }
  // 要素数を取得します。

  IPhonemeData At(int index);
  // 指定したインデックスの要素を取得します。
  // 引数：
  //   index - インデックス。
  // 戻り値：
  //   要素。

  IPhonemeDataArray Duplicate();
  // 配列を複製します。
  // 戻り値：
  //   複製した配列のインスタンス。

}

// 音素データの単位オブジェクト。
interface IPhonemeData
{

  string Phoneme { get; }
  // 音素を取得します。

  double StartTime { get; }
  // 開始時間を取得します。単位は秒。

  double EndTime { get; }
  // 終了時間を取得します。単位は秒。

}

// 文字列の配列を表すオブジェクト。
interface IStringArray
{

  int Length { get; }
  // 要素数を取得します。

  string At(int index);
  // 指定したインデックスの要素を取得します。
  // 引数：
  //   index - インデックス
  // 戻り値：
  //   要素。

  IStringArray Duplicate();
  // 配列を複製します。
  // 戻り値：
  //   複製した配列のインスタンス。

}

// EOF
