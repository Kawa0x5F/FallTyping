/*
 * HandyGraphicを利用して上から落ちてくる文字をタイプするゲームを作成する
 * 快適かつ、スコアなどが表示されるものを目標とする
 * 2023/06/29 Kawa_09
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <handy.h>

#define WND_WIDTH 600
#define WND_HEIGHT 600
#define KANA_NUM 85
#define SMALL_KANA_NUM 16
#define FINISH_TYPING 2

/* ------ 構造体の宣言 ------*/
// 文字列の管理をする構造体
typedef struct{
    int canDraw; // 描画したかどうかを保持する変数
    double x; // 描画時のx座標を保持する変数
    double y; // 描画時のy座標を保持する変数
    int inNum; // 何文字まで入力されたのかを保存する変数
    char origin[256]; // 落とす文字列を保存する配列
    char kana[256]; // 落とす文字列の仮名を保存する配列
    char example[128]; // ローマ字の入力例を保存する配列
}Str;

/* ------ プロトタイプ宣言 ------ */
int random_string_index(int strNum, Str *strings); // 文字列の個数内の乱数を返す関数
void set_string_example(Str *strings, int strIndex); // ローマ字で入力の例を作り、セットする関数
int get_japanese_index(char *str, int charIndex); // 対応している日本語の文字を、対応するローマ字が保存されている配列の添え字を返す
int check_input_char(Str *strings, int strIndex, unsigned int ch); // 入力された文字の正誤判定をし、場合によって入力例を書き換える

/* ------ グローバル変数の宣言 ------*/
// 母音を保管する配列
char vowel[][2] = {"a","i","u","e","o"};

// 子音を保管する配列
char consonant[][3][4] = {{""},{"k"},{"s"},{"t"},{"n"},
                          {"h"},{"m"},{"y"},{"r"},{"g"},
                          {"z"},{"d"},{"b"},{"p"},{"v"},{"w"},{"wy"},
                          {"l","x"},{"ly"},{"lt"},{"lw","xw"},
                          {"n","nn"}};

// 拗音がくるパターンを保存する二次元配列
int youon[KANA_NUM][SMALL_KANA_NUM];

// 拗音に対応するための文字列を保管する配列
char youonStr[][3][4] = {{""},{"y"},{"w"},{"h"},{"y","h"},
                         {"w","q"},{"f"},{"f","y"},{"v"},{"wh"},
                         {"w","wh"}};

// ひらがなと伸ばし棒のデータを保管する配列
char japaneseStr[] = "あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやいゆえよらりるれろ"
                     "がぎぐげござじずぜぞだぢづでどばびぶべぼぱぴぷぺぽあいゔえおわいうえをあゐうゑおぁぃぅぇぉゃぃゅぇょぁぃっぇぉゎぃぅぇぉんー";

/* ---------------------- */
/* ------ メイン処理 ------ */
/* ---------------------- */
int main() {

    /* ------ HandyGraphic関係の変数の宣言 ------ */
    int titleLayerId; // タイトル用のレイヤのidを保存する変数
    double romajiX,romajiY; // 入力例文字列の描画範囲を保存するための変数
    doubleLayer doubleLayerId; // ダブルレイヤ変数の宣言
    hgevent *eventCtx = NULL; // HgEventの返り値のポインタを保存するhgevent型ポインタ変数

    /* ------ タイピングの処理用の変数の宣言 ------ */
    int strNum = 0; // 落とす文字列の数を保存する変数
    int strIndex; // 落とす文字列の配列の番号を保存する変数
    Str *strings = NULL; // 文字列の情報を保持する構造体

    /* ------ ゲームのシステムに関係する変数の宣言 ------ */
    int level = 0; // 難易度を表す仮の変数

    /* ------ ファイルポインタの宣言 ------ */
    FILE *fpInYouon; // 拗音がくるパターンのあるファイル用のポインタ
    FILE *fpInString; // 落とす文字列のあるファイル用のポインタ
    FILE *fpInStringKana; // 落とす文字列の仮名のあるファイル用のポインタ

    /* ------ 構造体のメモリを動的に確保する ------ */
    strings = (Str*) malloc(100 * sizeof(Str));

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
        if(fscanf(fpInString, "%s", strings[i].origin) == EOF){
            break;
        }
        fscanf(fpInStringKana, "%s",strings[i].kana);
        strings[i].canDraw = 0;
        strings[i].x = 200.0;
        strings[i].y = 600.0;
        strings[i].inNum = 0;
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
    if((strIndex = random_string_index(strNum, strings)) == -1){
        printf("これ以上出力できる文字列がありません\n");
        strIndex = 0;
    }
    // 入力例文字列の設定
    set_string_example(strings,strIndex);

    // キー入力が得られるようにマスクを設定
    HgSetEventMask(HG_KEY_DOWN);
    for(;;) { // 無限ループ
        // 表示するレイヤに関する処理
        int layerId = HgLSwitch(&doubleLayerId);
        HgLClear(layerId); // レイヤの描画を削除する

        // 落とす場所もすでに落としている文字列に被らないようにランダムに決める
        /* ------ 描画 ------ */
        // 画面の装飾
        HgSetColor(HG_RED);
        HgLine(0, 150, 600, 150);
        // 文字列の描画
        HgSetColor(HG_BLACK);

        // 入力が終わっていなかったら文字列を描画する
        if (strings[strIndex].canDraw != 2) {
            HgWText(layerId, 200, strings[strIndex].y, strings[strIndex].origin); // 文字列を描画
            HgWSetFont(layerId, HG_M, 50);
            HgWTextSize(layerId, &romajiX, &romajiY, strings[strIndex].example); // 入力例文字列の描画範囲を取得
            HgWText(layerId, WND_WIDTH / 2.0 - romajiX / 2.0, 150 / 2.0 - romajiY / 2.0,
                    strings[strIndex].example); // 文字列の描画
            HgWSetFont(layerId, HG_M, 30);
        }

        // 落ちてくる文字がラインについたら終了
        if(strings[strIndex].y < 150){
            break;
        }

        // 動かす
        strings[strIndex].y -= 1.0; // 仮の文字列を仮の速度で下に落とす
        // 入力の常時受けとり
        eventCtx = HgEventNonBlocking(); // イベントを取得する
        if(eventCtx != NULL){// イベントがあった時
            if(eventCtx->type == HG_KEY_DOWN){ // イベントがキー入力の時
                printf("%d\n", eventCtx->ch);
                // 正誤判定とそれの反映の準備
                check_input_char(strings,strIndex,eventCtx->ch);
                // タブキーで入力する文字を選択
            }
        }

        // 今洗濯している文字列が入力終了しているかを判定
        if(strings[strIndex].inNum == strlen(strings[strIndex].example)){
            // スコアの処理
            // 描画を終了する
            strings[strIndex].canDraw = 2;
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

/* ---------------------- */
/* ------ ユーザ関数 ------ */
/* ---------------------- */

// これまで表示されていない文字列配列のindexをランダムに返す
int random_string_index(int strNum, Str *strings){
    int canCheck = 0; // 表示できる文字列が残っているかどうかを保持する変数
    int random; // 乱数を保存する変数

    for(int i = 0; i < strNum; i++){
        if(strings[i].canDraw == 0){ // 表示できる文字列があった時
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
    }while(strings[random].canDraw == 1);
    strings[random].canDraw = 1; // 選んだのでマークする

    return random;
}

// ローマ字で入力例を作る・変更する関数
void set_string_example(Str *strings, int strIndex){
    int len = strlen(strings[strIndex].kana);
    int nowCharIndex;
    int nextCharIndex = -1;
    int youonNum;
    char tmp[100] = "";

    printf("%s %d\n",strings[strIndex].kana, len);
    for(int i = 0; i < len; i+=3){
        nowCharIndex = get_japanese_index(strings[strIndex].kana,i);
        if(i+3 < len) nextCharIndex = get_japanese_index(strings[strIndex].kana,i+3);
        // もし次の文字が小書き文字かつ今の文字が拗音になる文字なら
        if(nextCharIndex >= 85 && nextCharIndex < 105)youonNum = youon[nowCharIndex][nextCharIndex-85];
        if(youonNum > 0 && nextCharIndex != 97){ // ようおんかつ次の文字が「っ」ではなかった時
            if(nowCharIndex != 27 && nowCharIndex != 72)strcat(tmp, consonant[nowCharIndex/5][0]); // 「ふ」じゃなかったら
            strcat(tmp,youonStr[youonNum][0]);

            i += 3;
            nowCharIndex = get_japanese_index(strings[strIndex].kana,i);
        }else{
            if(nowCharIndex == 97) { // 「っ」でかつ、次の文字があった時の処理
                printf("a");
                printf("%d",nextCharIndex);
                if(0 <= nextCharIndex && nextCharIndex < 5){
                    strcat(tmp, consonant[nowCharIndex / 5][0]);
                }else {
                    printf("b");
                    i += 3;
                    nowCharIndex = get_japanese_index(strings[strIndex].kana,i);
                    strcat(tmp, consonant[nowCharIndex / 5][0]);
                    strcat(tmp, consonant[nowCharIndex / 5][0]);
                }
            }
            else if(nowCharIndex == 105) { // 「ん」だった時の処理
                // 次の文字が母音もしくはや行だった時のみ、nを二つ表示する
                if (0 <= nextCharIndex && nextCharIndex < 5 ||
                    35 <= nextCharIndex && nextCharIndex < 40 ||
                    nextCharIndex == -1) {
                    strcat(tmp, consonant[nowCharIndex / 5][0]);
                    strcat(tmp, consonant[nowCharIndex / 5][0]);
                } else {
                    strcat(tmp, consonant[nowCharIndex / 5][0]);
                }
            }else if(nowCharIndex == 106){
                strcat(tmp, "-");
            }else {
                strcat(tmp, consonant[nowCharIndex / 5][0]);
            }
        }

        if(nowCharIndex != 105 && nowCharIndex != 106)strcat(tmp,vowel[nowCharIndex%5]);
        strcat(strings[strIndex].example,tmp);
        for(int j = 0; j < strlen(tmp); j++){
            tmp[j] = '\0';
        }
        nextCharIndex = -1;
        youonNum = -1;
    }
}

// 対応している日本語の文字を、対応するローマ字が保存されている配列の添え字を返す
int get_japanese_index(char *str, int charIndex){

    for(int i = 0; i < strlen(japaneseStr); i+=3){
        if(strncmp(&str[charIndex], &japaneseStr[i], 3) == 0){
            return i/3;
        }
    }
    return -1;
}

// 入力された文字の正誤判定を行い、入力例と違うが間違いでない入力だった時に入力例を書き換える。
int check_input_char(Str *strings, int strIndex, unsigned int ch) {
    if(strings[strIndex].example[strings[strIndex].inNum] == ch){
        strings[strIndex].inNum++;
        return 0;
    }

    return -1;
}