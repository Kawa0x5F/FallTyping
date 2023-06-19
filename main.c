/*
 * HandyGraphicを利用して上から落ちてくる文字をタイプするゲームを作成する
 * 快適かつ、スコアなどが表示されるものを目標とする
 * 2023/06/14 Kawa_09
 */

#include <stdio.h>
#include <handy.h>

#define WIDWIDTH 600
#define WIDHEIGH 600

// FallTypingのmain関数
int main() {

    int titleLayerId = 0; // タイトル用のレイヤのidを保存する変数
    doubleLayer doubleLayerId; // ダブルレイヤ変数の宣言
    int level = 0; // 難易度を表す仮の変数
    char str[20] = "あいうえお";// 仮の落とす文字列
    hgevent *eventCtx; // HgEventの返り値のポインタを保存する

    // Windowを開く
    HgOpen(WIDWIDTH,WIDHEIGH);

    // タイトル用のレイヤを追加する
    titleLayerId = HgWAddLayer(0);

    // タイトルの描画
    // タイトルのデザイン用の設定、描画をする
    HgWSetFont(titleLayerId,HG_M,50); // フォントとサイズを設定
    HgWText(titleLayerId,165,500,"Fall Typing"); // タイトルの文字列を描画
    HgWBox(titleLayerId,200,100,200,100);
    HgWBox(titleLayerId,200,225,200,100);
    HgWBox(titleLayerId,200,350,200,100);

    // マウスのクリックを検知し、ゲームモードを設定する
    HgSetEventMask(HG_MOUSE_DOWN); // イベントマスクをマウスのクリックで設定する

    // levelの値が0の間ループする
    do {
        eventCtx = HgEvent(); // イベントを取得する

        // 描画されたボックスの位置をクリックした時、難易度を設定する
        if(200 <= (*eventCtx).x && (*eventCtx).x <= 400){
            if(350 <= (*eventCtx).y && (*eventCtx).y <= 450){
                level = 1;
            }else if(225 <= (*eventCtx).y && (*eventCtx).y <= 325){
                level =2;
            }else if(100 <= (*eventCtx).y && (*eventCtx).y <= 200){
                level = 3;
            }
        }
    }while(level == 0);
    printf("難易度%dが選択されました\n", level);
    printf("x: %lf y: %lf\n", (*eventCtx).x, (*eventCtx).y);

    // タイトルレイヤを非表示にする
    HgClear();

    // テキストファイルの読み込み

    // ダブルレイヤを作成する
    doubleLayerId = HgWAddDoubleLayer(0);

    // 文字列のy座標方向の位置
    double strLocate = 600.0;

    // 無限ループ
    for(;;){
        // 表示するレイヤに関する処理
        int layerId = HgLSwitch(&doubleLayerId);
        HgLClear(layerId); // レイヤの描画を削除する

        /*
         * まだ選ばれていないテキストをランダムに選択
         * 速度と文字数から次に出す難易度を算出する
         */
        // 落とす場所もすでに落としている文字列に被らないようにランダムに決める
        // 描画
        HgWText(layerId,200,strLocate,"%s",str); // 仮の文字列を描画
        // 落ちてくる文字が地面についたら終了
        if(strLocate < 0){
            break;
        }
        // 動かす
        strLocate -= 1.0; // 仮の文字列を仮の速度で下に落とす
        printf("仮の文字列の位置: %0.3f\n",strLocate);
        // 入力の常時受けとり
        // 正誤判定とそれの反映
        // 矢印キーで入力する文字を選択
        // 落下スピードも調整してもいいかも？
        // 裏でスコアの計算
        // レベル（難易度）の概念を持たせて、場の単語の数を管理する
        HgSleep(0.02); // 少し処理を止める
    }
    //--------------------------
    // 結果と最終スコアを表示
    // 終わり

    // Windowを閉じる
    HgClose();

    return 0;
}
