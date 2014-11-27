float t;

void setup()
{
  size(800,800);
  background(0,0,0);
  noStroke();
  t=0.01;
}

void draw()
{
  //background(0);
  fill(0,10);
  rect(0,0,width,height);  
  pushMatrix();
  translate(width/2,height/2);
  float r = 0.8*width/2;
  float x = r*cos(-4*t);
  float y = r*sin(-3*t+PI/6);
  float xdelta = random(100);
  float ydelta = random(100);
  translate(x+xdelta,y+ydelta);
  fill(255,255,200);
  ellipse(0,0,20,20);
  t+=0.01;
  popMatrix();
}
