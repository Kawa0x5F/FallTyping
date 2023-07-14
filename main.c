/*
 * HandyGraphicを利用した上から落ちてくる文字をタイプするゲーム
 *
 * 2023/07/15 Kawa_09
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <handy.h>

#define WND_WIDTH 1000.0
#define WND_HEIGHT 800.0
#define KANA_NUM 85
#define SMALL_KANA_NUM 16
#define SMALL_KANA_FIRST_NUM 85
#define SMALL_KANA_LAST_NUM 105
#define JPN_CHAR_U 2
#define JPN_CHAR_KU 7
#define JPN_CHAR_SI 11
#define JPN_CHAR_TI 16
#define JPN_CHAR_HU 27
#define JPN_CHAR_ZI 51
#define JPN_CHAR_VU 72
#define JPN_CHAR_LTU 97
#define JPN_CHAR_NN 105
#define JPN_CHAR_BAR 106
#define WAIT_TYPING 0
#define DO_TYPING 1
#define FINISH_TYPING 2

/* ------ 構造体の宣言 ------*/
// 文字列の管理をする構造体
typedef struct{
    int canDraw;            // 描画したかどうかを保持する変数
    double x;               // 描画時のx座標を保持する変数
    double y;               // 描画時のy座標を保持する変数
    int inNum[4];           // 何文字まで入力されたのかを保存する変数
                            // [0]:全体の入力文字数
                            // [1]:日本語で一文字分遅れた全体の文字数
                            // [2]:日本語での文字数
                            // [3]:[2]の文字数の中での入力文字数
    char origin[256];       // 落とす文字列を保存する配列
    char kana[256];         // 落とす文字列の仮名を保存する配列
    char example[128];      // ローマ字の入力例を保存する配列
    char input[128];        // 入力された文字列を保存する配列
    char wait[20][10][128]; // 入力待ちの文字のパターンを保存する配列
    double nowTime;         // 文字列が落ち始めてからの時間を保存する変数
    double startTime;       // 文字列が落ち始めた時間を保存する変数
    double endTime;         // 文字列が落ち終わる時間を保存する変数
}Str;

/* ------ プロトタイプ宣言 ------ */
double random_x_location(Str *strings, int strIndex, int layerId); // ランダムにx座標を決めて、その値を返す関数
int random_string_index(int strNum, Str *strings); // 文字列の個数内の乱数を返す関数
void set_string_example(Str *strings, int strIndex); // ローマ字で各文字と全文の入力例をセットする関数
void change_string_example(Str *strings, int strIndex); // 入力例を変更する関数
int set_char_pattern(Str *strings, int strIndex, int charIndex, int japaneseCharIndex); // 文字の入力パターンをセットする関数
int get_japanese_index(char *str, int charIndex); // 対応している日本語の文字を、対応するローマ字が保存されている配列の添え字を返す
int check_input_char(Str *strings, int strIndex, unsigned int ch); // 入力された文字の正誤判定をし、場合によって入力例を書き換える

/* ------ グローバル変数の宣言 ------*/
// 母音を保管する配列
char vowel[][2] = {"a","i","u","e","o"};

// 子音を保管する配列
char consonant[][3][4] = {{""},{"k"},{"s","sh"},{"t","ch"},{"n"},
                          {"h","f"},{"m"},{"y"},{"r"},{"g"},
                          {"z","j"},{"d"},{"b"},{"p"},{"v"},{"w"},{"wy"},
                          {"x","l"},{"xy","ly"},{"lt"},{"lw","xw"},
                          {"n","nn"}};

// 拗音がくるパターンを保存する二次元配列
int youon[KANA_NUM][SMALL_KANA_NUM];

// 拗音に対応するための文字列を保管する配列
char youonStr[][3][4] = {{""},{"y"},{"w"},{"h"},{"y","h"},
                         {"w","q"},{"f"},{"f","y"},{"v"},{"wh"},
                         {"wh","w"}};

// ひらがなと伸ばし棒のデータを保管する配列
char japaneseStr[] = "あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやいゆえよらりるれろ"
                     "がぎぐげござじずぜぞだぢづでどばびぶべぼぱぴぷぺぽあいゔえおわいうえをあゐうゑおぁぃぅぇぉゃぃゅぇょぁぃっぇぉゎぃぅぇぉんー";

/* ---------------------- */
/* ------ メイン処理 ------ */
/* ---------------------- */
int main() {

    /* ------ HandyGraphic関係の変数の宣言 ------ */
    doubleLayer doubleLayerId; // ダブルレイヤ変数の宣言
    hgevent *eventCtx = NULL; // HgEventの返り値のポインタを保存するhgevent型ポインタ変数

    /* ------ タイトル画面用の変数の宣言 ------ */
    char titleStr[] = "Fall Typing"; // タイトルの文字列を保存する配列
    char titleBoxStr[3][25] = {"Easy","Normal","Difficult"}; // タイトルのボックスに表示する文字列を保存する配列
    int titleLayerId; // タイトル用のレイヤのidを保存する変数
    double titleMainFontSize; // タイトルのゲーム名のフォントサイズを指定する変数
    double titleComponentFontSize; // タイトルのコンポーネントのフォントサイズを指定する変数
    double titleStrX,titleStrY; // タイトル文字列の描画範囲を保存するための変数
    double titleBoxFloor, titleBoxX, titleBoxWidth; // タイトルに表示するボックスの位置と大きさを決めるための変数
    double titleGap; // タイトルのボックスの大きさ、間隔を決めるための変数

    /* ------ タイピングの処理用の変数の宣言 ------ */
    int strNum = 0; // 落とす文字列の数を保存する変数
    int strIndex = -1; // 落とす文字列の配列の番号を保存する変数
    int endLine = WND_WIDTH / 4; // 文字列が当たると終了の線の位置を表す変数
    double romajiStrX,romajiStrY,romajiCharX,romajiCharY; // 入力例文字列の描画範囲を保存するための変数
    double drawCharLocationX = 0; // 文字描画の位置を保存するための変数
    Str *strings = NULL; // 文字列の情報を保持する構造体

    /* ------ ゲームのシステムに関係する変数の宣言 ------ */
    int level = 0; // 難易度を表す変数
    int touchEndLine = 0; // 当たった場合終了となる線に当たったかどうかを保持する変数 0 : 当たっていない 1 : 当たった
    double fallSpeed = 0; // 落下速度を表す変数
    int finishTypingNum; // ゲーム終了に必要なタイピング完了文字列数を保存する変数
    int completeTypingNum = 0; // タイピングが完了した文字列の数を保存する変数
    int fallStrNum[30]; // 落下中の文字列の番号を保存する配列
    int fallStrNumIndex = 0; // fallStrNumの有効な要素の数を保存する変数
    int flag = 0; // フラグを必要とする処理用の変数
    double nowTime = 0; // ゲーム開始からの経過時間を保存する変数
    double tmpTime; // 一時的に現在の時間を保存する変数
    double beforeTime; // 経過時間を保存する処理で前ループの時との差分を取るための変数
    double beforeFallTime; // １つ前の文字列を落下させ始めた時間を保存する変数
    double fallInterval; // 文字列を落下させ始める時間の間隔を保存する変数
    struct timeval timeCtx; // 時間を保存する構造体

    /* ------ ファイルポインタの宣言 ------ */
    FILE *fpInYouon; // 拗音がくるパターンのあるファイル用のポインタ
    FILE *fpInString; // 落とす文字列のあるファイル用のポインタ
    FILE *fpInStringKana; // 落とす文字列の仮名のあるファイル用のポインタ


    /* --------------------------------------- */
    /* ------------ ゲームの処理開始 ------------ */
    /* --------------------------------------- */

    // 判定に使う変数の初期化
    for(int i = 0; i < 30; i++){
        fallStrNum[i] = -1;
    }
    srand((unsigned int)time(NULL)); // 乱数の初期化


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
        strings[i].canDraw = WAIT_TYPING;
        strings[i].y = WND_HEIGHT;
        strings[i].inNum[0] = 0;
        strings[i].inNum[1] = 0;
        strings[i].inNum[2] = 0;
        strings[i].inNum[3] = 1;
        strNum++; // 文字列の数を数える
        strings[i].nowTime = -1;
        strings[i].startTime = 0;
        for(int j = 0; j < 10; j++){
            sprintf(strings[i].wait[i][j] ,"%c", '\0');
        }
        set_string_example(strings,i); // 入力例をセット
    }
    // Windowを開く
    HgOpen(WND_WIDTH,WND_HEIGHT);


    /* ------ タイトル画面の描画 ------ */
    // タイトル用のレイヤを追加する
    titleLayerId = HgWAddLayer(0);

    // タイトルのデザイン用の設定、描画をする
    // タイトルの文字列の描画、設定
    // タイトルのフォントサイズはウィンドウの縦横の小さい方に合わせる
    WND_WIDTH <= WND_HEIGHT ? (titleMainFontSize = WND_WIDTH / 10) : (titleMainFontSize = WND_HEIGHT / 10);
    HgWSetFont(titleLayerId,HG_M,titleMainFontSize);
    HgWTextSize(titleLayerId, &titleStrX, &titleStrY, titleStr); // タイトル文字列の描画範囲を取得
    HgWText(titleLayerId, WND_WIDTH / 2 - titleStrX / 2, WND_HEIGHT / 6 * 5, titleStr);

    // タイトルのボックスの描画、その設定
    titleBoxFloor = WND_HEIGHT / 6; // ボックスの最下の座標を設定
    titleBoxX = WND_WIDTH / 3; // ボックスのX座標を設定
    titleBoxWidth = WND_WIDTH / 3; // ボックスの横幅を設定
    titleGap = (WND_HEIGHT / 3 * 2) / 16; // ボックスの間隔を設定
    titleComponentFontSize = titleGap * 1.5;
    HgWBox(titleLayerId, titleBoxX, titleBoxFloor, titleBoxWidth, titleGap * 4);
    HgWBox(titleLayerId, titleBoxX, titleBoxFloor + titleGap *  5, titleBoxWidth, titleGap * 4);
    HgWBox(titleLayerId, titleBoxX, titleBoxFloor + titleGap * 10, titleBoxWidth, titleGap * 4);
    HgWSetFont(titleLayerId,HG_M,titleComponentFontSize);
    HgWTextSize(titleLayerId, &titleStrX, &titleStrY, titleBoxStr[0]);
    HgWText(titleLayerId, WND_WIDTH / 2 - titleStrX / 2,
            (titleBoxFloor + titleGap * 12) - titleStrY / 2, titleBoxStr[0]);
    HgWTextSize(titleLayerId, &titleStrX, &titleStrY, titleBoxStr[1]);
    HgWText(titleLayerId, WND_WIDTH / 2 - titleStrX / 2,
            (titleBoxFloor + titleGap * 7) - titleStrY / 2, titleBoxStr[1]);
    HgWTextSize(titleLayerId, &titleStrX, &titleStrY, titleBoxStr[2]);
    HgWText(titleLayerId, WND_WIDTH / 2 - titleStrX / 2,
            (titleBoxFloor + titleGap * 2) - titleStrY / 2, titleBoxStr[2]);

    // マウスのクリックを検知し、ゲームモードを設定する
    HgSetEventMask(HG_MOUSE_DOWN); // イベントマスクをマウスのクリックで設定する

    // levelの値が0の間ループする
    do {
        eventCtx = HgEvent(); // イベントを取得する

        // 描画されたボックスの位置をクリックした時、難易度を設定する
        if(titleBoxX <= (*eventCtx).x && (*eventCtx).x <= titleBoxX + titleBoxWidth){
            if(titleBoxFloor + titleGap * 10 <= (*eventCtx).y && (*eventCtx).y <= titleBoxFloor + titleGap * 14) {
                level = 1;
                fallSpeed = 50.0;
                fallInterval = 2;
                finishTypingNum = 10;
            }else if(titleBoxFloor + titleGap * 5 <= (*eventCtx).y && (*eventCtx).y <= titleBoxFloor + titleGap * 9) {
                level = 2;
                fallSpeed = 100;
                fallInterval = 1;
                finishTypingNum = 15;
            }else if(titleBoxFloor  <= (*eventCtx).y && (*eventCtx).y <= titleBoxFloor + titleGap * 4) {
                level = 3;
                fallSpeed = 150.0;
                fallInterval = 0.8;
                finishTypingNum = 20;
            }
        }
    }while(level == 0);
    printf("難易度%dが選択されました\n", level);
    printf("x: %lf y: %lf\n", (*eventCtx).x, (*eventCtx).y);

    // タイトルレイヤを非表示にする
    HgClear();

    // ダブルレイヤを作成する
    doubleLayerId = HgWAddDoubleLayer(0);

    // キー入力が得られるようにマスクを設定
    HgSetEventMask(HG_KEY_DOWN);
    // ----------------------------------------------------------------------------------------------
    // ゲームのメインループ
    // ----------------------------------------------------------------------------------------------
    // 難易度ごとの回数で文字列を入力し終えるまで、もしくは当たったら終わりの線に当たるまでループする
    while(completeTypingNum < finishTypingNum && touchEndLine != 1) {

        /* ------ レイヤ処理 ------ */
        int layerId = HgLSwitch(&doubleLayerId);
        HgLClear(layerId); // レイヤの描画を削除する

        /* ------ 時間の取得 ------ */
        gettimeofday(&timeCtx, NULL);
        tmpTime = (double)timeCtx.tv_usec * 0.000001;
        if(tmpTime < beforeTime) {
            nowTime += (1 - beforeTime) + tmpTime;
            beforeTime = tmpTime;
        }else {
            nowTime += tmpTime - beforeTime;
            beforeTime = tmpTime;
        }
        // 文字列の落ちている時間を更新する
        for(int i = 0; i < fallStrNumIndex; i++){
            if(fallStrNum[i] == -1)break; // 落ちている文字列がなくなったらループを抜ける
            strings[fallStrNum[i]].nowTime = nowTime - strings[fallStrNum[i]].startTime;
        }

        // 落とす場所もできるだけすでに落としている文字列に被らないようにランダムに決める
        /* ------ 新たに文字列を落とす処理 ------ */
        if(fallInterval < nowTime - beforeFallTime || fallStrNumIndex == 0){
            fallStrNum[fallStrNumIndex] = random_string_index(strNum, strings);
            fallStrNumIndex++;
            beforeFallTime = nowTime;
            if(strIndex == -1){
                strIndex = fallStrNum[0];
            }
            int indexNum = fallStrNum[fallStrNumIndex - 1];
            // 文字列を落とすために必要な初期化をする
            strings[indexNum].nowTime = 0;
            strings[indexNum].startTime = nowTime;
            strings[indexNum].endTime = (WND_HEIGHT - endLine) / fallSpeed;
            strings[indexNum].x = random_x_location(strings, strIndex, layerId);
            strings[indexNum].canDraw = DO_TYPING;
        }

        /* ------ 描画 ------ */
        // 画面の装飾
        HgSetColor(HG_RED);
        HgLine(0, endLine, WND_WIDTH, endLine);
        // 文字列の描画
        HgSetColor(HG_BLACK);

        // 落ちてくる文字列の描画
        for(int i = 0; i < fallStrNumIndex; i++){
            if(fallStrNum[i] == -1)break; // 落ちている文字列がなくなったらループを抜ける
            int indexNum = fallStrNum[i];
            HgWText(layerId, strings[indexNum].x, strings[indexNum].y, strings[indexNum].origin); // 文字列を描画
        }

        // 入力が終わっていなかったら入力例の文字列を描画する
        if (strings[strIndex].canDraw != 2) {
            HgWSetFont(layerId, HG_M, 50);
            HgWTextSize(layerId, &romajiStrX, &romajiStrY, strings[strIndex].example); // 入力例文字列の描画範囲を取得
            for(int i = 0; i < strlen(strings[strIndex].example); i++){
                HgWTextSize(layerId,&romajiCharX, &romajiCharY, "%c", strings[strIndex].example[i]);
                if(i < strings[strIndex].inNum[0]){
                    HgWSetColor(layerId, HG_ORANGE);
                }else{
                    HgWSetColor(layerId, HG_BLACK);
                }
                HgWText(layerId, WND_WIDTH / 2.0 - romajiStrX / 2.0 + drawCharLocationX, 150 / 2.0 - romajiStrY / 2.0,
                        "%c" , strings[strIndex].example[i]); // 文字列の描画
                drawCharLocationX += romajiCharX;
            }
            drawCharLocationX = 0;
            HgWSetFont(layerId, HG_M, 30);
        }


        /* ------ 文字列の位置を更新 ------ */
        for(int i = 0; i < fallStrNumIndex; i++){
            if(fallStrNum[i] == -1)break; // 落ちている文字列がなくなったらループを抜ける
            int indexNum = fallStrNum[i];
            if(strings[indexNum].y < endLine){ // 落ちている文字列が当たったらダメな線に当たっていたら終了のフラグを立てる
                touchEndLine = 1;
                break;
            }
            if(strings[indexNum].canDraw != FINISH_TYPING) { // 文字列を難易度ごとの速度で下に落とす
                strings[indexNum].y = (double)(strings[indexNum].nowTime - strings[indexNum].endTime) * -fallSpeed + endLine;
            }
            printf("%d %f %f %f\n", indexNum, strings[strIndex].y, strings[strIndex].nowTime, strings[strIndex].endTime);
        }

        // 入力の常時受けとり
        eventCtx = HgEventNonBlocking(); // イベントを取得する
        if(eventCtx != NULL){// イベントがあった時
            if(eventCtx->type == HG_KEY_DOWN){ // イベントがキー入力の時
                // 正誤判定とそれの反映の準備
                check_input_char(strings,strIndex,eventCtx->ch);
                if(strings[strIndex].example[strings[strIndex].inNum[0]-1] != strings[strIndex].input[strings[strIndex].inNum[0]-1]){
                    // 入力された文字と入力例が違い時、入力例を作り直す
                    change_string_example(strings,strIndex);
                }
                // タブキーで入力する文字を選択
            }
        }

        // 今選択している文字列が入力終了しているかを判定
        if(strlen(strings[strIndex].example) == strlen(strings[strIndex].input) && strings[strIndex].canDraw == 1){
            // 終わった時
            // スコアの処理
            completeTypingNum += 1; // 入力が終わった文字列数のカウント
            strings[strIndex].canDraw = FINISH_TYPING; // 描画を終了する
            // 落ちている文字列の番号を保存している配列から、入力の終わった文字列の番号を消す
            for(int i = 0; i < fallStrNumIndex; i++){
                if(fallStrNum[i] == strIndex)flag = 1; // 落ち終わった文字列が合った時にフラグを立てる
                if(flag == 1)fallStrNum[i] = fallStrNum[i+1]; // 落ち終わった文字列以降の文字列を一つずつ前にずらす
            }
            fallStrNumIndex--; // 落ちている文字列の数を減らす
            if(0 < fallStrNumIndex)strIndex = fallStrNum[0]; // 次に入力する文字列の番号をセットする

        }
        // スコアの計算
        // レベル（難易度）の概念を持たせて、場の単語の数を管理する
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

/**
 * これまで表示されていない文字列配列のindexをランダムに返す
 *
 * @param strings 文字列とそれに関する情報を保存する構造体
 * @param strIndex 文字列の番号
 *
 * @return x座標の位置
 */
double random_x_location(Str *strings, int strIndex, int layerId){
    double x,y;
    double random; // 乱数を保存する変数

    // テキストを描画した時の幅を調べる
    HgWTextSize(layerId,&x, &y, strings[strIndex].origin);

    // ランダムにこれまで表示していない文字列の番号を探す
    random = (double)(rand() % (int)(WND_WIDTH - x)); // 0 ~ (WND_WIDTH) までの乱数を出力

    return random;
}

/**
 * これまで表示されていない文字列配列のindexをランダムに返す
 *
 * @param strNum 文字列の数
 * @param strings 文字列とそれに関する情報を保存する構造体
 *
 * @return 文字列の番号
 */
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
    do{
        random = rand()% strNum; // 0 ~ strNum までの乱数を出力
    }while(strings[random].canDraw == 1);
    strings[random].canDraw = 1; // 選んだのでマークする

    return random;
}

/**
 * ローマ字で各文字の入力例と全文の入力例をセットする関数
 *
 * @param strings 文字列とそれに関する情報を保存する構造体
 * @param strIndex 文字列の番号
 **/
void set_string_example(Str *strings, int strIndex){
    int len = (int)strlen(strings[strIndex].kana); // 文字列の長さを保存する変数
    int nowCharIndex; // 文字の番号を保存する変数
    int nextCharIndex; // 一つ先の位置の文字の番号を保存する変数
    int charArrayIndex; // 文字のセットされている配列の数を保存する変数
    int youonNum = -1; // 拗音のパターンを表す変数

    // 文字のバイト数が3なので、3ずつプラスしてループする
    /*
     * nowCharIndex / 5 : 母音の数を割ることで、 子音の番号と合わせる
     * nextCharIndex % 5: 剰余算をする事で、母音の番号と合わせる
     * */
    printf("%s %d\n", strings[strIndex].kana, len);
    for(int i = 0, k = 0; i < len; i+=3, k++){
        nowCharIndex = get_japanese_index(strings[strIndex].kana,i);
        charArrayIndex = set_char_pattern(strings,strIndex,k,nowCharIndex); // 文字の入力パターンをセットする
        if(i+3 > len)break; // 次の文字がない時は終了
        nextCharIndex = get_japanese_index(strings[strIndex].kana,i+3);
        if(nextCharIndex >= SMALL_KANA_FIRST_NUM && nextCharIndex < SMALL_KANA_LAST_NUM) { // 次の文字が小書き文字なら
            // 添字の番号を調整して、拗音のパターンの数字を代入
            youonNum = youon[nowCharIndex][nextCharIndex - SMALL_KANA_FIRST_NUM];
        }

        if(youonNum > 0 && nextCharIndex != JPN_CHAR_LTU){ // youonNum > 0 : 拗音であることを表す
            if(nowCharIndex == JPN_CHAR_U) {
                sprintf(strings[strIndex].wait[k][charArrayIndex], "%s%s%c", youonStr[youonNum][0],
                        vowel[nextCharIndex % 5], '*');
                if(youonNum == 10){
                    charArrayIndex++;
                    sprintf(strings[strIndex].wait[k][charArrayIndex], "%s%s%c", youonStr[youonNum][1],
                            vowel[nextCharIndex % 5], '*');
                }
            }else if(nowCharIndex == JPN_CHAR_KU){
                sprintf(strings[strIndex].wait[k][charArrayIndex], "%s%s%s%c", consonant[nowCharIndex / 5][0],
                        youonStr[youonNum][0], vowel[nextCharIndex % 5],'*');
                charArrayIndex++;
                sprintf(strings[strIndex].wait[k][charArrayIndex], "%s%s%c", youonStr[youonNum][1],
                        vowel[nextCharIndex % 5],'*');
            }else if(nowCharIndex == JPN_CHAR_TI) {
                sprintf(strings[strIndex].wait[k][charArrayIndex], "%s%s%s%c", consonant[nowCharIndex / 5][0],
                        youonStr[youonNum][0], vowel[nextCharIndex % 5],'*');
                if (youonNum == 4) {
                    charArrayIndex++;
                    sprintf(strings[strIndex].wait[k][charArrayIndex], "%s%s%c", consonant[nowCharIndex / 5][1],
                            vowel[nextCharIndex % 5], '*');
                }
            }else if(nowCharIndex == JPN_CHAR_SI){
                sprintf(strings[strIndex].wait[k][charArrayIndex], "%s%s%s%c", consonant[nowCharIndex / 5][0],
                        youonStr[youonNum][0], vowel[nextCharIndex % 5],'*');
                charArrayIndex++;
                sprintf(strings[strIndex].wait[k][charArrayIndex], "%s%s%s%c", consonant[nowCharIndex / 5][0],
                        youonStr[youonNum][1], vowel[nextCharIndex % 5],'*');
            }else if(nowCharIndex == JPN_CHAR_HU || nowCharIndex == JPN_CHAR_VU){
                if(youonNum == 6 || youonNum == 8){
                    sprintf(strings[strIndex].wait[k][charArrayIndex], "%s%s%c", youonStr[youonNum][0],
                            vowel[nextCharIndex % 5],'*');
                }else if(youonNum == 7){
                    sprintf(strings[strIndex].wait[k][charArrayIndex], "%s%s%c", youonStr[youonNum][0],
                            vowel[nextCharIndex % 5],'*');
                    charArrayIndex++;
                    sprintf(strings[strIndex].wait[k][charArrayIndex], "%s%s%s%c", consonant[nowCharIndex / 5][0],
                            youonStr[youonNum][1], vowel[nextCharIndex % 5],'*');
                }else{
                    sprintf(strings[strIndex].wait[k][charArrayIndex], "%s%s%s%c", consonant[nowCharIndex / 5][0],
                            youonStr[youonNum][0], vowel[nextCharIndex % 5],'*');
                }
            }else if(nowCharIndex == JPN_CHAR_ZI){
                sprintf(strings[strIndex].wait[k][charArrayIndex], "%s%s%s%c", consonant[nowCharIndex / 5][0],
                        youonStr[youonNum][0], vowel[nextCharIndex % 5],'*');
                charArrayIndex++;
                sprintf(strings[strIndex].wait[k][charArrayIndex], "%c%s%c", 'j', vowel[nextCharIndex % 5],'*');
            }else{
                sprintf(strings[strIndex].wait[k][charArrayIndex], "%s%s%s%c", consonant[nowCharIndex / 5][0],
                        youonStr[youonNum][0], vowel[nextCharIndex % 5],'*');
            }
            charArrayIndex++;
        }else if(nowCharIndex == JPN_CHAR_LTU) {
            if(5 < nextCharIndex){ // 次の文字が母音意外だった時
                sprintf(strings[strIndex].wait[k][charArrayIndex], "%c%s%s%c",
                        consonant[nextCharIndex / 5][0][0], consonant[nextCharIndex / 5][0],vowel[nextCharIndex % 5],'*');
            }
        }else if(nowCharIndex == JPN_CHAR_NN) {
            printf("%d \n", nextCharIndex);
            // 次の文字があり、母音でないかつ、な行、や行ではなかった時、「n」をセットする
            if ( 5 <= nextCharIndex &&
                (nextCharIndex < 20 || nextCharIndex >= 25) &&
                (nextCharIndex < 35 || nextCharIndex >= 40) && nextCharIndex != JPN_CHAR_NN) {
                sprintf(strings[strIndex].wait[k][1], "%s", "n");
            }
        }
        youonNum = -1;

    }

    int j;
    for(int i = 0; i < len; i ++){
        for(j = 0; j < 10; j++){
            if(strings[strIndex].wait[i][j][0] == '\0')break;
            printf("%d:%s\n", j,strings[strIndex].wait[i][j]);
        }
        sprintf(strings[strIndex].example, "%s%s", strings[strIndex].example, strings[strIndex].wait[i][j-1]);
        if(strings[strIndex].example[strlen(strings[strIndex].example)-1] == '*'){
            strings[strIndex].example[strlen(strings[strIndex].example)-1] = '\0';
            i++;
        }
    }
}

/**
 * 文字列の入力パターンを変更する関数
 *
 * @param strings 文字列とそれに関する情報を保存する構造体
 * @param strIndex 文字列の番号
 */
void change_string_example(Str *strings, int strIndex){
    char tmp[10] = ""; // 一時的に文字列を保存する変数
    char exampleStr[10] = ""; // 表示する文字列を保存する変数
    int stayCharIndex = strings[strIndex].inNum[1];
    int jpnCharIndex = strings[strIndex].inNum[2];
    int jpnCharArrIndex = strings[strIndex].inNum[3];
    int len = (int)strlen(strings[strIndex].kana);

    sprintf(strings[strIndex].example, "%s", strings[strIndex].input); // 入力済みの文字列で初期化する
    int j;
    for(int i = jpnCharIndex; i < len; i ++){
        for(j = 0; j < 10; j++){
            if(strings[strIndex].wait[i][j][0] == '\0')break;
            sprintf(tmp, "%s", &strings[strIndex].input[stayCharIndex]);
            if(strncmp(tmp, strings[strIndex].wait[i][j], jpnCharArrIndex-1) == 0){
                printf("aaa\n");
                sprintf(exampleStr, "%s", &strings[strIndex].wait[i][j][jpnCharArrIndex-1]);
            }
            printf("change string %d %d : %s %s\n", i, jpnCharArrIndex, tmp, strings[strIndex].wait[i][j]);
            printf("exampleStr : %s\n", exampleStr);
        }
        if(exampleStr[0] == '\0')sprintf(exampleStr, "%s", strings[strIndex].wait[i][j-1]);
        sprintf(strings[strIndex].example, "%s%s", strings[strIndex].example, exampleStr);
        if(strings[strIndex].example[strlen(strings[strIndex].example)-1] == '*'){
            strings[strIndex].example[strlen(strings[strIndex].example)-1] = '\0';
            i++;
        }
        sprintf(exampleStr, "%s", "");
    }
}

/**
 * 指定された日本語の１文字の入力パターンを作成する
 * 「っ」は「ltu」のパターンのみを作成する
 * 「ん」は「nn」のパターンのみを作成する
 *
 * @param strings 文字列とそれに関する情報を保存する構造体
 * @param strIndex 文字列の番号
 * @param charIndex 文字列の何文字目かを指定する番号
 * @param japaneseCharIndex 日本語の文字のを指定する変数
 *
 * @return セットされた配列の数を返す
 *
 */
int set_char_pattern(Str *strings, int strIndex, int charIndex, int japaneseCharIndex) {

    if(0 <= japaneseCharIndex && japaneseCharIndex < 5){
        sprintf(strings[strIndex].wait[charIndex][0], "%s", vowel[japaneseCharIndex]);
        return 1;
    }else if(SMALL_KANA_FIRST_NUM <= japaneseCharIndex && japaneseCharIndex < SMALL_KANA_LAST_NUM ||
       japaneseCharIndex == JPN_CHAR_SI ||
       japaneseCharIndex == JPN_CHAR_TI ||
       japaneseCharIndex == JPN_CHAR_HU ||
       japaneseCharIndex == JPN_CHAR_ZI) {
        sprintf(strings[strIndex].wait[charIndex][0], "%s%s", consonant[japaneseCharIndex / 5][0], vowel[japaneseCharIndex % 5]);
        sprintf(strings[strIndex].wait[charIndex][1], "%s%s", consonant[japaneseCharIndex / 5][1], vowel[japaneseCharIndex % 5]);
        return 2;
    }else if(japaneseCharIndex == JPN_CHAR_NN) {
        sprintf(strings[strIndex].wait[charIndex][0], "%s", consonant[japaneseCharIndex / 5][1]);
        return 1;
    }else if(japaneseCharIndex == JPN_CHAR_BAR) {
        strcat(strings[strIndex].wait[charIndex][0], "-");
        return 1;
    }else{
        sprintf(strings[strIndex].wait[charIndex][0], "%s%s", consonant[japaneseCharIndex / 5][0], vowel[japaneseCharIndex % 5]);
        return 1;
    }
}

/**
 * 指定された日本語の文字の番号を返す
 *
 * @param str 日本語の文字が保存されている配列
 * @param charIndex 日本語の文字を指定する番号
 *
 * @return 日本語の文字の番号を返す
 */
int get_japanese_index(char *str, int charIndex){

    for(int i = 0; i < strlen(japaneseStr); i+=3){
        if(strncmp(&str[charIndex], &japaneseStr[i], 3) == 0){
            return i/3;
        }
    }
    return -1;
}

/**
 * 入力された文字の正誤判定を行う。
 *
 * @param strings 文字列とそれに関する情報を保存する構造体
 * @param strIndex 文字列の番号
 * @param ch 入力されたアルファベット一文字
 *
 * @return 0:成功 -1:失敗 で入力の正誤を返す
 */
int check_input_char(Str *strings, int strIndex, unsigned int ch) {

    int stayCharIndex = strings[strIndex].inNum[1];
    int jpnCharIndex = strings[strIndex].inNum[2];
    int jpnCharArrIndex = strings[strIndex].inNum[3];
    char tmp[10] = ""; // 一時的に文字列を保存する変数
    char tmpY[10] = ""; // 拗音用の一時的に文字列を保存する変数

    // 「n」がすでに一文字入力されていて、「n」以外の文字が入力された時
    // 「n」一文字だけで入力を終了できるか判定する
    if(ch != 'n' && jpnCharArrIndex == 2){
        if(strcmp(strings[strIndex].wait[jpnCharIndex][1], "n") == 0){
            strings[strIndex].inNum[1]++;
            strings[strIndex].inNum[2]++;
            strings[strIndex].inNum[3] = 1;
            stayCharIndex = strings[strIndex].inNum[1];
            jpnCharIndex = strings[strIndex].inNum[2];
            jpnCharArrIndex = strings[strIndex].inNum[3];
        }
    }

    for(int i = 0; i < 10; i++){
        if(strings[strIndex].wait[jpnCharIndex][i][0] == '\0')break;
        sprintf(tmp, "%s%c", &strings[strIndex].input[stayCharIndex], ch);
        sprintf(tmpY, "%s%c%c", &strings[strIndex].input[stayCharIndex], ch, '*');
        printf("%s\n", tmp);
        if(strncmp(tmp, strings[strIndex].wait[jpnCharIndex][i], jpnCharArrIndex) == 0 ||
           strncmp(tmpY, strings[strIndex].wait[jpnCharIndex][i], jpnCharArrIndex) == 0){
            sprintf(strings[strIndex].input, "%s%c", strings[strIndex].input, ch);
            strings[strIndex].inNum[0]++;
            strings[strIndex].inNum[3]++;
            printf("正解\n");
            if(strings[strIndex].wait[jpnCharIndex][i][jpnCharArrIndex] == '\0'){
                strings[strIndex].inNum[1] += (int)strlen(strings[strIndex].wait[jpnCharIndex][i]);
                strings[strIndex].inNum[2]++;
                strings[strIndex].inNum[3] = 1;
            }else if(strings[strIndex].wait[jpnCharIndex][i][jpnCharArrIndex] == '*'){
                printf("a\n");
                strings[strIndex].inNum[1] += (int)strlen(strings[strIndex].wait[jpnCharIndex][i]) - 1;
                strings[strIndex].inNum[2] += 2;
                strings[strIndex].inNum[3] = 1;
            }
            return 0;
        }
    }

    return -1;
}