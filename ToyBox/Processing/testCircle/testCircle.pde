float t; //時間

void setup()
{
  size(800,800);
  background(0,0,0);
  noStroke();
  t=0;
}

void draw()
{
  //background(0);
  //background(0)の代わりに半透明(アルファが10/255)な
  //四角形を(0,0)-(width,height)の大きさで塗ると残像のような効果
  fill(0,50);
  rect(0,0,width,height);
  
  pushMatrix(); //popMatrix()と対
  translate(width/2,height/2); //原点を中心に移動
  float r = 0.8*width/2;
  
  // 円の位置を時間を変数にリサジュー図形で決めている。
  // x=r*cos(t)とy=r*sin(t)なら円になる
  float x = r*cos(-4*t);
  float y = r*sin(-3*t+PI/6);
  
  float xdelta = 0; //この行をコメントアウトしてrandomを有効にするとブレる
  //float xdelta = random(100);
  
  float ydelta = 0; //この行をコメントアウトしてrandomを有効にするとブレる
  //float ydelta = random(100);
  
  translate(x+xdelta,y+ydelta); //円を書く場所に移動
  fill(255,255,200); //円の色
  ellipse(0,0,20,20); //translateを使っているので常に原点に円を書くだけで良い
  t+=0.01;
  popMatrix();
}
