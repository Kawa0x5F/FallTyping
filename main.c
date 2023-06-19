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

    int level = 0; // 難易度を表す仮の変数
    char str[20] = "あいうえお";// 仮の落とす文字列
    hgevent *eventCtx; // イベント構造体の受け取り用

    // タイトル用のWindowを開く
    HgOpen(WIDWIDTH,WIDHEIGH);

    // タイトルの描画
    // タイトルのデザイン用の設定、描画をする
    HgSetFont(HG_M,50); // フォントとサイズを設定
    HgText(165,500,"Fall Typing"); // タイトルの文字列を描画
    HgBox(200,100,200,100);
    HgBox(200,225,200,100);
    HgBox(200,350,200,100);

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

    // Windowの要素を全消去する
    HgClear();
    // テキストファイルの読み込み
    // 無限ループ
//    for(;;){
//        /*
//         * まだ選ばれていないテキストをランダムに選択
//         * 速度と文字数から次に出す難易度を算出する
//         */
//        // 落とす場所もすでに落としている文字列に被らないようにランダムに決める
//        // 描画
//        // 動かす
//        // 入力の常時受けとり
//        // 正誤判定とそれの反映
//        // 矢印キーで入力する文字を選択
//        // 落下スピードも調整してもいいかも？
//        // 裏でスコアの計算
//        // レベル（難易度）の概念を持たせて、場の単語の数を管理する
//        // 落ちてくる文字が地面についたら終了
//    }
    //--------------------------
    // 結果と最終スコアを表示
    // 終わり

    // Windowを閉じる
    HgClose();

    return 0;
}
