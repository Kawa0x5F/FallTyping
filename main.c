/*
 * HandyGraphicを利用して上から落ちてくる文字をタイプするゲームを作成する
 * 快適かつ、スコアなどが表示されるものを目標とする
 * 2023/06/14 Kawa_09
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <handy.h>

#define WND_WIDTH 600
#define WND_HEIGHT 600
#define KANA_NUM 72
#define SMALL_KANA_NUM 9

/* ------ プロトタイプ宣言 ------ */
int random_string_index(int strNum, int strBool[]);


// FallTypingのmain関数
int main() {

    /* ------ HandyGraphic関係の変数の宣言 ------ */
    int titleLayerId; // タイトル用のレイヤのidを保存する変数
    doubleLayer doubleLayerId; // ダブルレイヤ変数の宣言
    hgevent *eventCtx = NULL; // HgEventの返り値のポインタを保存するhgevent型ポインタ変数

    /* ------ タイピングの処理用の変数の宣言 ------ */
    int strNum = 0; // 落とす文字列の数を保存する変数
    int strIndex; // 落とす文字列の配列の番号を保存する変数
    double strLocate = 600.0; // 文字列のy座標の位置
    int youon[KANA_NUM][SMALL_KANA_NUM]; // 拗音がくるパターンを保存する二次元配列
    int strBool[100]; // 文字列を表示したかどうかを保持する配列
    char str[100][256];// 落とす文字列を保存する二次元配列
    char strKana[100][256]; // 落とす文字列の仮名を保存する二次元配列

    /* ------ ゲームのシステムに関係する変数の宣言 ------ */
    int level = 0; // 難易度を表す仮の変数

    /* ------ ファイルポインタの宣言 ------ */
    FILE *fpInYouon; // 拗音がくるパターンのあるファイル用のポインタ
    FILE *fpInString; // 落とす文字列のあるファイル用のポインタ
    FILE *fpInStringKana; // 落とす文字列の仮名のあるファイル用のポインタ

    /* ------- テキストファイルの読み込み ------- */
    // 拗音のパターンのあるファイルを開く
    if((fpInYouon = fopen("./../youon.txt","r")) == NULL){
        printf("ファイルのオープンに失敗しました\nyouon.txtがあるかを確認してください\n");
        exit(0);
    }
    if((fpInString = fopen("./../string.txt","r")) == NULL){
        printf("ファイルのオープンに失敗しました\nstring.txtがあるかを確認してください\n");
        exit(0);
    }
    if((fpInStringKana = fopen("./../string_kana.txt","r")) == NULL){
        printf("ファイルのオープンに失敗しました\nstring_kana.txtがあるかを確認してください\n");
        exit(0);
    }
    // 拗音のパターンをファイルから取得
    for(int i = 0; i < KANA_NUM; i++)for(int j = 0; j < SMALL_KANA_NUM; j++)fscanf(fpInYouon,"%d", &youon[i][j]);
    // 落とす文字列とその仮名をファイルから取得
    for(int i = 0; i < 100; i++){
        if(fscanf(fpInString,"%s", &str[i]) == EOF){
            break;
        }
        fscanf(fpInStringKana,"%s",&strKana[i]);
        strBool[i] = 0; // 文字列を表示したかを保持する配列の初期化
        strNum++; // 文字列の数を数える
    }

    // Windowを開く
    HgOpen(WND_WIDTH,WND_HEIGHT);

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

    // ダブルレイヤを作成する
    doubleLayerId = HgWAddDoubleLayer(0);
    /*
     * まだ選ばれていないテキストをランダムに選択
    * 速度と文字数から次に出す難易度を算出する
    */
    if((strIndex = random_string_index(strNum,strBool)) == -1){
        printf("これ以上出力できる文字列がありません\n");
        strIndex = 0;
    }

    // キー入力が得られるようにマスクを設定
    HgSetEventMask(HG_KEY_DOWN);
    for(;;){ // 無限ループ
        // 表示するレイヤに関する処理
        int layerId = HgLSwitch(&doubleLayerId);
        HgLClear(layerId); // レイヤの描画を削除する

        // 落とす場所もすでに落としている文字列に被らないようにランダムに決める
        /* ------ 描画 ------ */
        // 画面の装飾
        HgSetColor(HG_RED);
        HgLine(0,150,600,150);
        // 文字列の描画
        HgSetColor(HG_BLACK);
        for(int i = 0; i < strlen(str[strIndex]); i+=3) {
            HgWText(layerId, 200 + i * 5, strLocate, "%c%c%c", str[strIndex][i],str[strIndex][i+1],str[strIndex][i+2]); // 文字列を描画
        }
        // 落ちてくる文字がラインについたら終了
        if(strLocate < 150){
            break;
        }
        // 動かす
        strLocate -= 1.0; // 仮の文字列を仮の速度で下に落とす
        // 入力の常時受けとり
        eventCtx = HgEventNonBlocking(); // イベントを取得する
        if(eventCtx != NULL){// イベントがあった時
            if(eventCtx->type == HG_KEY_DOWN){ // イベントがキー入力の時
                // 正誤判定とそれの反映の準備

                // タブキーで入力する文字を選択
            }
        }
        // 落下スピードも調整してもいいかも？
        // スコアの計算
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

// これまで表示されていない文字列配列のindexをランダムに返す
int random_string_index(int strNum, int strBool[]){
    int canCheck = 0; // 表示できる文字列が残っているかどうかを保持する変数
    int random; // 乱数を保存する変数

    for(int i = 0; i < strNum; i++){
        if(strBool[i] == 0){ // 表示できる文字列があった時
            canCheck = 1; // check変数に値を入れて、ループを抜ける
            break;
        }
    }
    // 表示できる文字列がなかった時、エラーを返す
    if(canCheck == 0){
        return -1;
    }

    // ランダムにこれまで表示していない文字列の番号を探す
    srand((unsigned int)time(NULL)); // 乱数の初期化
    do{
        random = rand()% strNum; // 0 ~ strNum までの乱数を出力
    }while(strBool[random] == 1);
    strBool[random] = 1; // 選んだのでマークする

    return random;
}