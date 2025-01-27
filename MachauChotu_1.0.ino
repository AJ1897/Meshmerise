#define mlf 23
#define mlb 22
#define mlp 11
#define mrf 26
#define mrb 27
#define mrp 10
#define ledr 2
#define ledg 4
#define ledb 5
#define ledc 3

byte mls = 150, mrs = 150, mlt = 170, mrt = 170;
int reading[7];
byte dread[7];
int err = 0, perr = 0, derr = 0, corr = 0, sen_thr = 500,mlpwm = 0, mrpwm = 0;
const byte  kp = 50, kd = 20, lt_ov = 100, rt_ov = 100, tback_ov=500;
long long int t = 0, tst = 0, t_back = 0, tl = 0, distance = 0;
const byte line_ov = 70;
int end_dry = 0, wsum = 0, sum = 0;
int D = 0;
int i = 1, j = 1, l = 0, ua = 0, uat = 0, cl = 0, xf = 0, yf = 0,ij=0,ifxy=0,dirf=0,prv_dir=0;
byte  z[50][50], vis[50][50];
int dir[50], x[50], y[50];
bool left, right, dead_end, all_white, node, turn, st, st_check, rt_check,tes = 0;

void initialise(){
for(int a = 0; a < 50; a++)
{
  for(int b = 0; b < 50; b++)
    vis[a][b] = 0;
}
}

void setup() {
  Serial.begin(9600);
  digitalWrite(ledr, HIGH);
  digitalWrite(ledg, HIGH);
  digitalWrite(ledb, HIGH);
  initialise();
  pinMode(mlf, OUTPUT);
  pinMode(mlb, OUTPUT);
  pinMode(mlp, OUTPUT);
  pinMode(mrp, OUTPUT);
  pinMode(mrf, OUTPUT);
  pinMode(mrb, OUTPUT);
  pinMode(ledr, OUTPUT);
  pinMode(ledg, OUTPUT);
  pinMode(ledb, OUTPUT);
  pinMode(ledc, OUTPUT);
  digitalWrite(ledc, HIGH);
  Serial.println("Reset done");
}

void sensor_read() {
  reading[0] = analogRead(A0);
  reading[1] = analogRead(A1);
  reading[2] = analogRead(A2);
  reading[3] = analogRead(A3);
  reading[4] = analogRead(A4);
  reading[5] = analogRead(A5);
  reading[6] = analogRead(A6);
  for (int l = 0; l < 7; l++)
  {
      if (reading[l] < sen_thr) dread[l] = 0;
      else dread[l] = 1;
  }
}

void calc_error() {
  sum = 0;
  for (int l = 0; l < 7; l++)
    sum = sum + dread[l];
  wsum = 7 * dread[0] + 5 * dread[1] + 2 * dread[2] - 2 * dread[4] - 5 * dread[5] - 7 * dread[6];
  err = wsum / sum;
}

void pause() {
  digitalWrite(mlb, LOW);
  digitalWrite(mrb, LOW);
  digitalWrite(mlf, LOW);
  digitalWrite(mrf, LOW);

  analogWrite(mlp, 0);
  analogWrite(mrp, 0);
}


void go() {
  digitalWrite(mlf, HIGH);
  digitalWrite(mrf, HIGH);
  digitalWrite(mlb, LOW);
  digitalWrite(mrb, LOW);

  analogWrite(mlp, mlpwm);
  analogWrite(mrp, mrpwm);
}

void update_node() { //checks which type of node it is
  //Serial.println("update node");
  digitalWrite(ledr, HIGH);
  digitalWrite(ledg, HIGH);
  digitalWrite(ledb, HIGH);
  sensor_read();
  calc_error();
  left = ((dread[0] == 1) && (dread[1] == 1) && (dread[2] == 1) && (dread[3] == 1) && (dread[6] == 0));
  right = ((dread[6] == 1) && (dread[5] == 1) && (dread[4] == 1) && (dread[3] == 1) && (dread[0] == 0));
  rt_check = (right);//right=false kyu nhi likha
  //left=((dread[0]==1)&&(dread[1]==1)&&(dread[2]==1)&&(dread[6]==0));
  //right=((dread[6]==1)&&(dread[5]==1)&&(dread[4]==1)&&(dread[0]==0));
  dead_end = (sum == 0);
  all_white = (sum == 7);
  node = (dead_end || all_white);
  turn = (left || right);
  st = false;
  st_check = (st);

  /*Serial.println("left Value: "+ left );
    Serial.println("right Value: "+ right );
    Serial.println("dead_end Value: "+ dead_end );
    Serial.println("all_white Value: "+ all_white );
    Serial.println("node Value: "+ node );
    Serial.println("turn Value: "+ turn );*/

  if (turn) {
    t = millis();
    while (millis() < t + 20) go();
    sensor_read();
    left = ((dread[0] == 1) && (dread[1] == 1) && (dread[2] == 1) && (dread[3] == 1) && (dread[6] == 0));
    right = ((dread[6] == 1) && (dread[5] == 1) && (dread[4] == 1) && (dread[3] == 1) && (dread[0] == 0));
    //if (!rt_check) rt_check = (right); //to be considered
    all_white = (sum == 7);
    turn = (left || right);
    node = (dead_end || all_white);
  }
  if (left && !right) {
    digitalWrite(ledr, LOW);
    while (dread[0])
    {
      go();
      sensor_read();
    }
    t = millis();
    while (millis() < t + 20) go();
    sensor_read();
    if (dread[3] || dread[4] || dread[5]) st_check = true;
    rt_check = false;
  }

  else if (!left && right)
  { rt_check = false;
    digitalWrite(ledg, LOW);
    while (dread[6])
    {
      go();
      sensor_read();
    }
    tst = millis();
    while (millis() < tst + 20) go();
    sensor_read();
    if (dread[3] || dread[4] || dread[5]) {
      st = true;
      right = false;
      rt_check = true;
    }
  }
  else if (dead_end) digitalWrite(ledb, LOW);
  else if (all_white) {
    rt_check = true;

    digitalWrite(ledr, LOW);
    digitalWrite(ledg, LOW);
    t = millis();
    while ((dread[0] && dread[6]))
    {
      if (millis() > t + 600) {
        end_dry = 1;
        digitalWrite(ledr, LOW);
        digitalWrite(ledg, LOW);
        digitalWrite(ledb, LOW);
        t = millis();
        while (millis() < t + 12000) pause();
      }
      go();
      sensor_read();
    } 
    //Added if condition
    if(!end_dry){
    if (dread[3] || dread[4] || dread[5]) st_check = true;
    }
    //t=millis();
    //while(millis()<t+200)
    //go();
  }
}


void motor() {
  derr = err - perr;
  corr = kp * err + kd * derr;
  perr = err;

  if (corr < 0) {
    mrpwm = mrs + corr;
    mlpwm = mls - corr;
  }
  else {
    mrpwm = mrs + corr;
    mlpwm = mls - corr;
  }
  if (mlpwm > 254) mlpwm = 254;
  else if (mrpwm > 254) mrpwm = 254;
  else if (mlpwm < 0) mlpwm = 0;
  else if (mrpwm < 0) mrpwm = 0;
  go();
}

void line_follow() {
  sensor_read();
  calc_error();
  motor();
}

void turn_left(int lnov) {
  //Serial.println("Turning left");
  t = millis();
  while (millis() < t + lnov)
  {
    digitalWrite(mlf, HIGH);
    digitalWrite(mrf, HIGH);
    digitalWrite(mlb, LOW);
    digitalWrite(mrb, LOW);

    analogWrite(mlp, mls);
    analogWrite(mrp, mrs);
  }
  sensor_read();
  while (!dread[0]) {//changed from 2 to 1
    digitalWrite(mrf, HIGH);
    digitalWrite(mlf, LOW);
    digitalWrite(mrb, LOW);
    digitalWrite(mlb, HIGH);

    analogWrite(mlp, mlt);
    analogWrite(mrp, mrt);
    sensor_read();
  }

  /*long long int tim =millis();
    while(millis()<tim+200)
    {
    digitalWrite(mrf,LOW);
    digitalWrite(mlf,LOW);
    digitalWrite(mrb,HIGH);
    digitalWrite(mlb,HIGH);

    analogWrite(mlp,mlt);
    analogWrite(mrp,mrt);
    }*/
}

void turn_right() {
  //Serial.println("Turning right");
  t = millis();
  while (millis() < t + line_ov) {
    digitalWrite(mlf, HIGH);
    digitalWrite(mrf, HIGH);
    digitalWrite(mlb, LOW);
    digitalWrite(mrb, LOW);

    analogWrite(mlp, mls);
    analogWrite(mrp, mrs);
  }

  sensor_read();
  while ((!dread[6])) {//changed from 6 to 4
    digitalWrite(mlf, HIGH);
    digitalWrite(mrf, LOW);
    digitalWrite(mlb, LOW);
    digitalWrite(mrb, HIGH);
    analogWrite(mlp, mlt);
    analogWrite(mrp, mrt);
    sensor_read();
  }
  /*long long int tim =millis();
    while(millis()<tim+300)
    {
    digitalWrite(mrf,LOW);
    digitalWrite(mlf,LOW);
    digitalWrite(mrb,HIGH);
    digitalWrite(mlb,HIGH);

    analogWrite(mlp,mlt+10);
    analogWrite(mrp,mrt);
    }
    line_follow();*/
}

void turn_back() {
  Serial.println("Turning back");
  t = millis();
  while (millis() < t + 400) {
    digitalWrite(mlf, HIGH);
    digitalWrite(mrf, HIGH);
    digitalWrite(mlb, LOW);
    digitalWrite(mrb, LOW);

    analogWrite(mlp, 0);
    analogWrite(mrp, 0);
  }

  t_back = millis();
  while (millis() < t_back + tback_ov) {
    digitalWrite(mlf, LOW);
    digitalWrite(mrf, HIGH);
    digitalWrite(mlb, HIGH);
    digitalWrite(mrb, LOW);

    analogWrite(mlp, mlt);
    analogWrite(mrp, mrt);
    sensor_read();
  }
  
  t_back = millis();
  while (!dread[2] || millis() < t_back + line_ov) {
    digitalWrite(mlf, LOW);
    digitalWrite(mrf, HIGH);
    digitalWrite(mlb, HIGH);
    digitalWrite(mrb, LOW);

    analogWrite(mlp, mlt);
    analogWrite(mrp, mrt);
    sensor_read();
  }
  long long int tim = millis();
  while (millis() < tim + 200)
  {
    digitalWrite(mrf, LOW);
    digitalWrite(mlf, LOW);
    digitalWrite(mrb, HIGH);
    digitalWrite(mlb, HIGH);

    analogWrite(mlp, mlt);
    analogWrite(mrp, mrt + 10);
  }
}

/*start*/
/* up=1
  left=2
  down=3
  right=4
*/
/*void LT_node(){
  if(dir[i-1]==4) dir[i]=1;
  else dir[i]=dir[i-1]+1;
  }
  void RT_node(){
  if(dir[i-1]==1) dir[i]=4;
  else dir[i]=dir[i-1]-1;
  }
  void tb_node(){
   if((dir[i-1]==1)||(dir[i-1]==2))
     dir[i]=dir[i-1]+2;
     else
      dir[i]=dir[i-1]-2;
     }

  bool test_loops(){
  }

  void test(){
   if((i>1)&&(abs(dir[i-1]-dir[i-2])==2)){
     dir[i-1]=0;
     dir[i-2]=0;
     i=i-2;}
   }*/

void LT_node() {
  if (dir[i-1] == +1) dir[i] = -2;
  else if (dir[i-1] == -2) dir[i] = -1;
  else if (dir[i-1] == -1) dir[i] = 2;
  else if (dir[i-1] == 2) dir[i] = 1;
  Serial.print("i value = ");
  Serial.print(i);
  Serial.print("Dir[i] element = ");
  Serial.println(dir[i]);
}

void RT_node() {
  if (dir[i-1] == 1) dir[i] = 2;
  else if (dir[i-1] == -2) dir[i] = 1;
  else if (dir[i-1] == -1) dir[i] = -2;
  else if (dir[i-1] == 2) dir[i] = -1;
  Serial.print("i value = ");
  Serial.print(i);
  Serial.print("Dir[i] element = ");
  Serial.println(dir[i]);
}

void tb_node() {
  dir[i] = -dir[i-1];
  Serial.print("i value = ");
  Serial.print(i);
  Serial.print("Dir[i] element = ");
  Serial.println(dir[i]);
}

void test() {
  if(dir[i-1] + dir[i-2] == 0) tes = 1;
  else tes=0;
  if (dir[i-3] + dir[i-2] == 0) {
    //x[i-3] = x[i-1];
    x[i-2] = x[i];
    x[i-1] = 0;
    x[i] = 0;
    //y[i-3] = y[i-1];
    y[i-2] = y[i];
    y[i-1] = 0;
    y[i] = 0;
    dir[i-3] = dir[i-1];
    dir[i-2] = dir[i];
    dir[i-1] = 0;
    dir[i] = 0;
  switch(z[x[i-3]][y[i-3]]){
  case 3:
  z[x[i-3]][y[i-3]]=0;
  break;
  case 2:
  z[x[i-3]][y[i-3]]=3;
  break;
  case 1:
  z[x[i-3]][y[i-3]]=0;
  break;
  }
    i = i-2;
  }
}

/*void test_loops(){
  dist[i]=(millis()-tl);
  //for(int j=i;j>0;j--){
  //distance=dist[j]*dir[j];
  // total_dist=total_dist+distance;
  //if(total_dist==0)
  //}
  for(int k=i;k>j;k--){dist[k]=0;
  dir[i]=0;}
  if(dir[i-1]==2){x[i]=x[i-1];
  y[i]=y[i-1]+dist[i];}
  else if(dir[i-1]==-2){x[i]=x[i-1];
  y[i]=y[i-1]-dist[i];}
  else if(dir[i-1]==1){x[i]=x[i-1]+dist[i];
  y[i]=y[i-1];}
  else if(dir[i-1]==-1){x[i]=x[i-1]-dist[i];
  y[i]=y[i-1];}

  for(int j=i-1;j>=0;j--){
  if((x[i]==x[j])&&(y[j]==y[i])){
  for(int k=j;k<=i;k++){
  dir[k]=0;
  x[k-1]=0;
  y[k-1]=0;
  }
  i=i-j;
  }
  else continue;}
  }*/

//check
void update_array() {
  if (dir[i-1] == 2) {
    x[i] = x[i-1];
    y[i] = y[i-1] + 1;
    
  }
  else if (dir[i-1] == -2) {
    x[i] = x[i-1];
    y[i] = y[i-1] - 1;
  }
  else if (dir[i-1] == 1) {
    x[i] = x[i-1] + 1;
    y[i] = y[i-1];
  }
  else if (dir[i-1] == -1) {
    x[i] = x[i-1] - 1;
    y[i] = y[i-1];
  }
  vis[x[i]][y[i]]=vis[x[i]][y[i]]+1;


  Serial.print("i value = ");
  Serial.print(i);
  Serial.print("   ");
  Serial.print("Current X[i] = ");
  Serial.print(x[i]);
  Serial.print("   ");
  Serial.print("Current Y[i] = ");
  Serial.println(y[i]);
}

int find_dir(int posi, int posf) {
  if ((posi - posf) == -1) return 1;
  else if ((posi - posf) == 1) return -1;
  else if ((posi - posf) == -10) return 2;
  else if ((posi - posf) == 10) return -2;
  else return 0;
}

void turnf(int dirf, int prv_dir) {
  switch (dirf) {
    case 2:
      if (prv_dir == 1) turn_right();
      else if (prv_dir == -1) turn_left(line_ov);
      else if (prv_dir == -2) turn_back();
      break;
    case 1:
      if (prv_dir == -2) turn_right();
      else if (prv_dir == 2) turn_left(line_ov);
      else if (prv_dir == -1) turn_back();
      break;
    case -2:
      if (prv_dir == -1) turn_right();
      else if (prv_dir == 1) turn_left(line_ov);
      else if (prv_dir == 2) turn_back();
      break;
    case -1:
      if (prv_dir == 2) turn_right();
      else if (prv_dir == -2) turn_left(line_ov);
      else if (prv_dir == 1) turn_back();
      break;
    /*case 0://bus
    if (prv_dir == 2) turn_left(line_ov);
      else if (prv_dir == -2) turn_right;
      else if (prv_dir == 1)  {t = millis();
      while (millis() < t + 250) line_follow();};
      else if (prv_dir == 0) turn_back;
      break;*/
  }
}

void follow_path(){
int h;
prv_dir=dir[i-1]; //bus
//bus
for (h = i; h > ifxy; h--){
  dirf = find_dir(10*y[h]+x[h],10*y[h-1]+x[h-1]);
  turnf(dirf, prv_dir);
  t=millis();
  while (!node && !turn && !st && millis()<t+1500)//bus
    {
    update_node();
    line_follow();
    //i++; //bus
    }
  prv_dir = dirf;
}
i=h;//bus
}


//-;-
//0 no opt
//1 only st
//2 st and right
//3 only right

void update_opt(int u) {
  if (!u) uat = 0;
  else {
    if (rt_check) {
      if (st_check) uat = 2;
      else uat = 3;
    }
    else if (st_check) uat = 1; //uat=3
    else uat = 0;
  }
  //z[x[i-1]][y[i-1]] = uat; bus
    z[x[i]][y[i]] = uat;
}

bool check_visited(int k) {
  if (vis[x[k]][y[k]] > 1) return 1;
  return 0;
}
int find_visited(int i){
  int h = i-1;
  while(10*y[h]+x[h] != 10*y[i]+x[i]) h=h-1;
  return h;
  }//bus
  
void check_loop() {
//  int ii = 0;
  if (check_visited(i) && !tes)
  {   int h = find_visited(i);
    //bus
    //while ((!((x[i] == x[ii]) && (y[i] == y[ii]))) && ii < i) ii++;
    //bus 
    switch(z[x[h]][y[h]])
    {
      case 0:
      ij = i-1;
      case 1:
      if (dir[h-1] == -dir[i-1]) { ij=i-1; z[x[h]][y[h]] = 0;}
      else ij=i;
      break;
      case 2:
      ij=i;
      case 3:
     // if (dir[h] == dir[i-1]) { ij=i-1; z[x[h]][y[h]] = 0;}
       ij=i-1;
      break;
     }
    //ij = i;//bus
    //ij=i-1;
    while (ij >= 0)
    {
      if (z[x[ij]][y[ij]] != 0)
      {
        Serial.println("z entry");
        xf = x[ij];
        yf = y[ij];
        ifxy=ij;
          //int l = i;
          follow_path();
        //0 no opt
        //1 only st
        //2 st and right
        //3 only right
        
        switch(z[xf][yf]){
          case 1:
          Serial.println("entry 1");
              z[xf][yf]=0;
              if(dir[i]==prv_dir) st=true;
              else if(dir[i]==-prv_dir) dead_end=true;
              else {
                switch(prv_dir){
                case 1:
                if(dir[i]==2) right=true;
                else left=true;
                break;
                case 2:
                if(dir[i]==1) left=true;
                else right=true;
                break;
                case -1:
                if(dir[i]==-2) right=true;
                else left=true;
                break;
                case -2:
                if(dir[i]==1) right=true;
                else left=true;
                break;
                }
                }
          case 2:
          Serial.println("entry 2");
              z[xf][yf]=3;
              if(dir[i]==prv_dir) st=true;
              else if(dir[i]==-prv_dir) dead_end=true;
              else {
                switch(prv_dir){
                case 1:
                if(dir[i]==2) right=true;
                else left=true;
                break;
                case 2:
                if(dir[i]==1) left=true;
                else right=true;
                break;
                case -1:
                if(dir[i]==-2) right=true;
                else left=true;
                break;
                case -2:
                if(dir[i]==1) right=true;
                else left=true;
                break;
                }
                }
            
          case 3:
          Serial.println("entry 3");
              z[xf][yf]=0;
              if(dir[i]==prv_dir) right=true;
              else if(dir[i]==-prv_dir) left=true;
              else {
                switch(prv_dir){
                case 1:
                if(dir[i]==2) dead_end=true;
                else st=true;
                break;
                case 2:
                if(dir[i]==1) st=true;
                else dead_end=true;
                break;
                case -1:
                if(dir[i]==-2) dead_end=true;
                else st=true;
                break;
                case -2:
                if(dir[i]==1) dead_end=true;
                else st=true;
                break;
                }
                }
          }
         //update_array();//bus 
      }
      else
      {
        ij = ij - 1;
        //i=i-1;
      }
    }
    
  }
  else
  {
    if(!tes)
    update_opt(1);
  }
}

void dry_run() {
  dir[0] = 1;
  //dist[0]=0;
  x[0]=0;
  y[0]=0;

//  if (i == 0) {
//    tl = millis();
//    i++;
//  }
  update_node();
  t = millis();
  ua=0;
    digitalWrite(ledr, HIGH);
    digitalWrite(ledg, HIGH);
    digitalWrite(ledb, HIGH);
  while (!node && !turn && !st) {
    if ((millis() > t + 600) && !ua) {
      Serial.println("Printing virtual node");
      digitalWrite(ledr, LOW);
      digitalWrite(ledb, LOW);
      update_array();
      if(i>2) test();
      check_loop();
      dir[i]=dir[i-1];
      i++;
      Serial.print("i value = ");
      Serial.println(i-1);
       Serial.print("Dir[i] elements : ");
       for(int chkarr=0;chkarr<i;chkarr++)
      {
        Serial.print(dir[chkarr]);
        Serial.print("  ");
      }
       Serial.println(" ");
      
      update_opt(0);
      ua++;
    }
    digitalWrite(ledr, HIGH);
    digitalWrite(ledg, HIGH);
    digitalWrite(ledb, HIGH);
    update_node();
    line_follow();
  }
  //ua = 0;
  if (node || turn || st) {
    update_array();
  if(check_visited(i)&&!z[x[i]][y[i]]){
    if(rt_check) {
      right=true;
      rt_check=false;
      left=false;
      st=false;
      }
      else if(st_check){
        st=true;
        st_check=false;
        left=false;
        }
    }
    
    if(i>2) test();
    check_loop();
    
    //update_array();
    //update_opt(1);
    //ua++;
    //Serial.println(dir[i-1]); //test();
    tl = millis();
    if (left) {
      turn_left(line_ov);
      LT_node();
      i++;
    }
    else if (st) {
      t = millis();
      while (millis() < t + 250) line_follow();
      dir[i] = dir[i-1];
      i++;
    }
    else if (right) {
      turn_right();
      RT_node();
      i++;
    }
    else if (dead_end) {
      turn_back();
      tb_node();
      i++;
    }
    else if (all_white) {
      t = millis();
      while (millis() < t + line_ov - 3) {
        digitalWrite(mlf, HIGH);
        digitalWrite(mrf, HIGH);
        digitalWrite(mlb, LOW);
        digitalWrite(mrb, LOW);
        analogWrite(mlp, mls);
        analogWrite(mrp, mrs);
      }
      update_node();
      /*if(all_white){
        //digitalWrite(led,HIGH);
        end_dry=1;
        t=millis();
        while(millis()<t+5000) pause();
        }
        else{
        turn_left(0);
        LT_node();
        i++;}*/
      if (!end_dry) {
        turn_left(0);
        LT_node();
        i++;
      }
    }
    Serial.print("i value = ");
  Serial.println(i-1);
  
 Serial.print("Dir[i] elements : ");
    for(int chkarr=0;chkarr<i;chkarr++)
    {
      
      Serial.print(dir[chkarr]);
      Serial.print("  ");
    }
    Serial.println(" ");
  }
  if (!end_dry) {
    t = millis();
    digitalWrite(ledr, HIGH);
    digitalWrite(ledg, HIGH);
    digitalWrite(ledb, HIGH);
    while (millis() < t + 400) {
      //digitalWrite(ledr, LOW);
      //digitalWrite(ledg, LOW);
      //digitalWrite(ledb, LOW);
      line_follow();
    }
    digitalWrite(ledr, HIGH);
    digitalWrite(ledg, HIGH);
    digitalWrite(ledb, HIGH);
  }
}

void follow_array() {
  if (((dir[l] == -2) && (dir[l-1] == 1)) || ((dir[l] == 2) && (dir[l-1] == -1)) || ((dir[l] == 1) && (dir[l-1] == 2)) || ((dir[l] == -1) && (dir[l-1] == -2))) turn_left(line_ov);
  else if (((dir[l-1] == -2) && (dir[l] == 1)) || ((dir[l-1] == 2) && (dir[l] == -1)) || ((dir[l-1] == 1) && (dir[l] == 2)) || ((dir[l-1] == -1) && (dir[l] == -2))) turn_right();
  else if (!(dir[l] - dir[l-1])) {
    t = millis();
    while (millis() < t + 250) line_follow();
  }
}

void main_run() {
  line_follow();
  update_node();
  t=millis();
  ua=0;
  while (!node && !turn && !st ) {
    if(millis()>t+600 && !ua){
      l++;
      ua++;
      }
    update_node();
    line_follow();
  }
  l++;
  follow_array();
  t = millis();
  while (millis() < t + 400) line_follow();
}
/*new
   up=+1
   left=-2
   down=-1
   right=+2
*/
void loop()
{
  if (end_dry != 1)
    dry_run();
  else
    main_run();
}
