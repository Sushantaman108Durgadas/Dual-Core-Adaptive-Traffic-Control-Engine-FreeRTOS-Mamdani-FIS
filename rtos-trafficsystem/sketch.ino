#include <ESP32Servo.h>
#include <cstdlib>
#include <ctime>

// ---------------- MAPS ---------------- //
int traffic_light_map[4][3] = {
  {2,0,4},
  {16,17,5},
  {18,19,21},
  {12,14,27}
};

int cam_servo_map[2] = {22,23};

const char dir[4] = {'E','N','W','S'};

// ---------------- GLOBAL DATA ---------------- //
float numvehicles;
float nextnumvehicles;

int width;
int length;

int defgreentime = 20000;
int defyellowtime = 5000;
int defredtime = 4 * (defgreentime + defyellowtime);

int greentime[4] = {20000,20000,20000,20000};
int yellowtime[4] = {5000,5000,5000,5000};
int redtime[4];

// ---------------- STRUCT ---------------- //
struct Camdetails
{
  int phaseshift;
  int camshift;
  char direction;
  int laneno;
  int numvehicles;
  int nextnumvehicles;
};

struct Camservobundlewrap
{
  Camdetails Camservodetails[2];
};

// ---------------- LANE CLASS ---------------- //
class Lane
{
private:
  int serialno;
  int greentime;
  int yellowtime;
  int redtime;
  int numvehicles;
  int nextnumvehicles;
  char laneorientation;

public:
  Lane(int serialno,int greentime,int yellowtime,int redtime)
  {
    this->serialno = serialno;
    this->greentime = greentime;
    this->yellowtime = yellowtime;
    this->redtime = redtime;

    for(int i=0;i<3;i++)
      pinMode(traffic_light_map[serialno][i],OUTPUT);

    laneorientation = dir[serialno];
    setredlight();
  }

  void setredlight()
  {
    digitalWrite(traffic_light_map[serialno][0],HIGH);
    digitalWrite(traffic_light_map[serialno][1],LOW);
    digitalWrite(traffic_light_map[serialno][2],LOW);
  }

  void setgreenlight()
  {
    digitalWrite(traffic_light_map[serialno][2],HIGH);
    digitalWrite(traffic_light_map[serialno][1],LOW);
    digitalWrite(traffic_light_map[serialno][0],LOW);
  }

  void setyellowlight()
  {
    digitalWrite(traffic_light_map[serialno][1],HIGH);
    digitalWrite(traffic_light_map[serialno][0],LOW);
    digitalWrite(traffic_light_map[serialno][2],LOW);
  }

  void traffic_control()
  {
    setgreenlight();
    vTaskDelay(pdMS_TO_TICKS(greentime));

    setyellowlight();
    vTaskDelay(pdMS_TO_TICKS(yellowtime));

    setredlight();
  }

  void update_vehiclenum(int numvehicles,int nextnumvehicles)
  {
    this->numvehicles = numvehicles;
    this->nextnumvehicles = nextnumvehicles;
  }

  void fuzzy_logic_controller()
  {
    if(nextnumvehicles > 50 && greentime > 0)
      greentime -= 2000;
    else if(nextnumvehicles < 25)
      greentime += 3000;

    if(numvehicles > 50)
      greentime += 5000;

    Serial.print("Green time for the signal ");
    Serial.print(serialno);
    Serial.print(" is");
    Serial.println(greentime);
  }
};

// ---------------- OBJECTS ---------------- //
Lane Traffic_lane[4] = {
  Lane(0,greentime[0],yellowtime[0],redtime[0]),
  Lane(1,greentime[1],yellowtime[1],redtime[1]),
  Lane(2,greentime[2],yellowtime[2],redtime[2]),
  Lane(3,greentime[3],yellowtime[3],redtime[3])
};

// ---------------- CAMERA CLASS ---------------- //
class CamServo
{
private:
  int camservopin;
  int serialno;
  Servo servo;
  Camservobundlewrap Camservobundle;

public:
  CamServo(int camservopin,int serialno)
  {
    this->camservopin = camservopin;
    this->serialno = serialno;

    for(int i=0;i<2;i++)
    {
      Camservobundle.Camservodetails[i].phaseshift = 90;
      Camservobundle.Camservodetails[i].camshift =
        sizeof(cam_servo_map)/sizeof(cam_servo_map[0]);
    }

    servo.attach(camservopin);
    servo.write(0);
  }

  void cam_capture()
  {
    for(int i=0;i<2;i++)
    {
      servo.write(i*Camservobundle.Camservodetails[i].phaseshift);
      vTaskDelay(pdMS_TO_TICKS(1000));
      Serial.println("Captured Image for Vehicle Count");

      fetch_numvehicles(i);
      fetch_nextnumvehicles(i);

      Camservobundle.Camservodetails[i].laneno =
        (serialno * Camservobundle.Camservodetails[i].camshift) + i;
    }
  }

  void fetch_numvehicles(int i)
  {
    Camservobundle.Camservodetails[i].numvehicles = rand() % 100;
  }

  void fetch_nextnumvehicles(int i)
  {
    Camservobundle.Camservodetails[i].nextnumvehicles = rand() % 100;
  }

  Camservobundlewrap getCamdata()
  {
    return Camservobundle;
  }
};

// ---------------- OBJECTS ---------------- //
CamServo CamServoModule[2] = {
  CamServo(cam_servo_map[0],0),
  CamServo(cam_servo_map[1],1)
};

// ---------------- FORWARD DECLARATIONS ---------------- //
void Traffic_Signal(void *pvParameters);
void Traffic_Monitoring(void *pvParameters);

// ---------------- SETUP ---------------- //
void setup()
{
  Serial.begin(115200);
  randomSeed(millis());

  for(int i=0;i<4;i++)
  {
    redtime[i] =
      greentime[(i+1)%4] + yellowtime[(i+1)%4] +
      greentime[(i+2)%4] + yellowtime[(i+2)%4] +
      greentime[(i+3)%4] + yellowtime[(i+3)%4];
  }

  xTaskCreate(Traffic_Signal, "Traffic", 4096, NULL, 1, NULL);
  xTaskCreate(Traffic_Monitoring, "Monitor", 4096, NULL, 1, NULL);
}

// ---------------- TASK 1 ---------------- //
void Traffic_Signal(void *pvParameters)
{
  while(true)
  {
    for(int i=0;i<4;i++)
    {
      Traffic_lane[i].traffic_control();
    }
  }
}

// ---------------- TASK 2 ---------------- //
void Traffic_Monitoring(void *pvParameters)
{
  while(true)
  {
    for(int i=0;i<2;i++)
    {
      CamServoModule[i].cam_capture();

      Camservobundlewrap Shareddata =
        CamServoModule[i].getCamdata();

      for(int k=0;k<2;k++)
      {
        int lane = Shareddata.Camservodetails[k].laneno;

        Traffic_lane[lane].update_vehiclenum(
          Shareddata.Camservodetails[k].numvehicles,
          Shareddata.Camservodetails[k].nextnumvehicles
        );

        Traffic_lane[lane].fuzzy_logic_controller();
      }
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// ---------------- LOOP ---------------- //
void loop()
{
  // FreeRTOS handles everything
}
