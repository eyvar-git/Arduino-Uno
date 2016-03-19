int SlPin=12; //назначение входного пина на который вешается кнопка запуска основного алгоритма

int EchoPin=10; //Echo-пин на УЗ-датчике
int TrigPin=9;  //Trig-пин на УЗ-датчике

float DIST=0;   //Дистанция
int CRS=0;      //Trig-пин на УЗ-датчике

int ServoPin=11;    //Сигнальный контакт сервопривода
int POS=0;          //Необходимое значение угла поворота.
int OLDPOS=0;       //Предыдущее значение угла поворота.

//далее идет назначение номеров выводов в соответствии с
//подключением к плате управления двигателями

int ENA=5;
int IN1=6;
int IN2=7;
int ENB=3;
int IN3=4;
int IN4=2;

//установка максимального и минимального значения ШИМ
//для ограничения напряжения питания двигателей.
//минимальные значения подбираем опытным путем,
//необходимо чтобы при этих значениях машинка смогла двигаться

int MinPWML=0;
int MinPWMR=0;
int MaxPWML=150;
int MaxPWMR=150;

int CD[9];
int DT=0;

void setup()
{
  pinMode(SlPin, INPUT);
  
  digitalWrite(SlPin, HIGH);
  pinMode(13, OUTPUT);
  
  pinMode(ServoPin, OUTPUT);
  
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  pinMode(TrigPin, OUTPUT);
  pinMode(EchoPin, INPUT);
  digitalWrite(EchoPin, LOW); 
}

void loop()
{//Проверка нажатия кнопки исполнения алгоритма.
  if(digitalRead(SlPin))
  {
// ---------------------------------
//  Начало кода основной программы.
// ---------------------------------
    
    DIST=sonar();
    //Serial.print("DIST = ");
    //Serial.println(DIST);
       
    if (DIST<20){
      
      roll('s');
      roll('b');
      delay(200);
      roll('s');
      
      CRS=course(); 
      
      rotate(CRS);
    
      ServoPos(90);
              
    }else{roll('f');;}
    
// --------------------------------
//  Конец кода основной программы.
// --------------------------------

  }else{
    
    OLDPOS=0;
    roll('s');
    ServoPos(90);

  }
}



//----------------------------------------------
//  Процедуры управления движением конструкции.
//----------------------------------------------

void steer(int l, int r) //установка направления вращения колес и скорости
{
  //задаем направление движения колес
  
  if(l>0)
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);    
  }else{
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);    
  }
  
  if(r>0)
  {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);    
  }else{
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);    
  }
  
  //рассчитываем абсолютные значения параметров скорости
  l=abs(l);
  r=abs(r);
  
  //ограничиваем значения входных параметров
  if(l>127){l=127;}
  if(r>127){r=127;}

  //подгоняем значения входных параметров под нужный диапазон
  l=map(l, 0, 127, MinPWML, MaxPWML);
  r=map(r, 0, 127, MinPWMR, MaxPWMR);
  
  // ограничиваем значения входных параметров, иначе при их низких значениях
  // на двигатели будет подаваться ток меньше, чем требуется для обеспечения
  // вращения колес.
  if(l<=MinPWML){l=0;}
  if(r<=MinPWMR){r=0;}  
  
  analogWrite(ENA, l);
  analogWrite(ENB, r);
}

void roll(int c) //указываем направление движения.
{
  switch(c)
  {
    case ('f'):
      steer(127, 127);
      break;
    case ('b'):
      steer(-127, -127);
      break;
    case ('s'):
      steer(0, 0);
      break;    
  } 
}

void rotate(int a) //поворот конструкции на заданный угол
{
  int t=0;
  int s=127; //скорость вращения колес.
  if (a<90)
  {
    t=(90-a)*4,16; //Число 4,16 - время поворота конструкции на 1 градус при данных аргументах steer(), подобрано опытным путем.
    steer(-s, s);
    delay(t);
    steer(0, 0);
  }else{
    
    t=(a-90)*4,16;
    steer(s, -s);
    delay(t);
    steer(0, 0);
  }
}

//--------------------------------------
//  Процедуры управления сервоприводом.
//--------------------------------------

void ServoPos(int angle)
{
  //Функция выставления движка сервопривода на заданный угол.
  //Параметр-значение угла.
  
  int PD;            //длительность управляющего импульса
  PD=2560-(angle)*11,11;
  
  //Производим рассчет количества циклов позиционирования.
  
  float DAN;         //Разность предыдующего и устанавливаемого значений угла.
  DAN=OLDPOS-angle;
  DAN=abs(DAN);
  OLDPOS=angle;
  int CP;           //Количество циклов позиционирования
  CP=DAN/1.8; //Оптимальное значение 1.8. Подбирается опытным путем.
  
  //посылаем управляющие импульсы   
  //Обеспечиваем сервопривод работой в зависимости
  //от количаства циклов позиционирования.
  
  for(int i=0; i<CP; i++) //i=100 for 180 degree
  {
    digitalWrite(ServoPin, HIGH); 
    delayMicroseconds(PD);
    digitalWrite(ServoPin, LOW);
    delayMicroseconds(23000);
  }
  

}
  
//------------------------------------------
//  Процедуры работы с датчиком расстояния.
//------------------------------------------

float sonar() //Измерение расстояния до обьекта.
{
  int cnt=0; //Значение счетчика количества запросов. 2 запроса.
  
  float c1=0;   //Переменные для хранения результатов замеров.
  float c2=0;
  
  repson:
  
  cnt++;
  //Посылаем стартовый импульс 10 мкс.
  digitalWrite(TrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(TrigPin, LOW);
  
  //Принимаем ответ

   float c=0;
         
      for(long t=0; t<40000; t=t+10)
      {
        if(digitalRead(EchoPin)){c=c+10;}
        delayMicroseconds(10);
      }
   
   if(cnt==1){c1=c; goto repson;}
   if(cnt==2){c2=c;}
   
   if(abs(c1-c2)>35000){c=38000;}else{c=(c1+c2)/2;}
   
   //Рассчитываем расстояние.
   float d=0;
   d=2*c/58;
   
   return d;
}  

int course()
  {
    int d=0;
    int dt=0;
    int cs=0;
    int p=0;
    
    for (p=0; p<7; p++)
    {
     ServoPos(p*30);
     dt=sonar();
     if (dt>d){d=dt; cs=p*30;}
    }
    
    return cs;
  }
