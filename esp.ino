#include <ESP8266WiFi.h>
#include <Ticker.h>
#define fpin D6
#define mpin D7

Ticker fticker, mticker;
typedef struct
{
  int ftime;
  int mtime;
} Info;
const String password = "1113";
String query[] = {"pass", "fri", "mot", "tf", "tm"};
Info data = {.ftime = 10, .mtime = -1};
WiFiServer server(80);

void handle_fridge() {
  Serial.print("fridge ticker running: ");
  Serial.println(data.ftime);
  if (data.ftime == 0)
  {
    //Turn on fridge
    digitalWrite(fpin, LOW);
    Serial.println("fridge on");
  }
  data.ftime--;
  if (data.ftime < 0)
  {
    fticker.detach();
  }
}

void handle_motor() {
  Serial.print("motor ticker running: ");
  Serial.println(data.mtime);
  if (data.mtime == 0)
  {
    //Turn off motor
    digitalWrite(mpin, HIGH);
    Serial.println("Motor off");
  }
  data.mtime--;
  if (data.mtime < 0)
  {
    mticker.detach();
  }
}


String getresponse()
{
  return
    String("{")
    + "\"f\": " + String(!digitalRead(fpin))
    + ",\"ft\": " + String(data.ftime)
    + ",\"m\": " + String(!digitalRead(mpin))
    + ",\"mt\": " + String(data.mtime)
    + "}\r\n";
}
void setup()
{
  Serial.begin(9600);
  Serial.println();
  pinMode(fpin, OUTPUT);
  pinMode(mpin, OUTPUT);
  digitalWrite(fpin, HIGH);
  digitalWrite(mpin, HIGH);

  fticker.attach(1, handle_fridge);


  WiFi.begin("TOP", "demopassword");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(2000);
    Serial.print(".");
  }
  server.begin();
  Serial.println("connected");
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}



void loop()
{
  WiFiClient client = server.available();
  // wait for a client (web browser) to connect
  if (client)
  {
    char method[3];
    char url[100];

    bool secured = true;
    bool extracted = false;
    Serial.println("\n[Client connected]");
    while (client.connected())
    {
      // read line by line what the client (web browser) is requesting

      if (client.available())
      {
        String line = client.readStringUntil('\r');
        Serial.print(line);
        if (!extracted)
        {
          sscanf(line.c_str(), "%3s %99s %*s", method, url);
          extracted = true;
          char *mid = strchr(url, '?');
          if (mid != nullptr) {
            //TODO parse query
            char *val = strtok(++mid, "&");
            while (val != nullptr) {
              if (secured)
              {
                if (strncmp(val, query[0].c_str(), query[0].length()) == 0)
                {
                  String tmp_val = val + query[0].length() + 1;
                  if (tmp_val == password)
                  {
                    secured = false;
                  }
                }
              } else {
                if (strncmp(val, query[1].c_str(), query[1].length()) == 0)
                {
                  String tmp_val = val + query[1].length() + 1;
                  int timer = tmp_val.toInt();
                  data.ftime = timer;
                  if (timer)
                  {
                    digitalWrite(fpin, HIGH);
                    fticker.attach(1, handle_fridge);
                  }
                }
                if (strncmp(val, query[2].c_str(), query[2].length()) == 0)
                {
                  String tmp_val = val + query[2].length() + 1;
                  int timer = tmp_val.toInt();
                  data.mtime = timer;
                  if (timer)
                  {
                    digitalWrite(mpin, LOW);
                    mticker.attach(1, handle_motor);
                  }
                }
                if (strncmp(val, query[3].c_str(), query[3].length()) == 0)
                {
                  String tmp_val = val + query[3].length() + 1;
                  int btn = tmp_val.toInt();
                  if (btn)
                  {
                    digitalWrite(fpin, !(digitalRead(fpin)));
                  }
                }
                if (strncmp(val, query[4].c_str(), query[4].length()) == 0)
                {
                  String tmp_val = val + query[4].length() + 1;
                  int btn = tmp_val.toInt();
                  if (btn)
                  {
                    digitalWrite(mpin, !(digitalRead(mpin)));
                  }
                }
              }
              val = strtok(nullptr, "&");
            }
          }
        } else {
          // wait for end of client's request, that is marked with an empty line
          if (line.length() == 1 && line[0] == '\n')
          {
            String response = getresponse();
            client.printf("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %u\r\n\r\n%s", response.length(), response.c_str());
            client.flush();
            Serial.println("responded");
            break;
          }
        }


      }

    }
    delay(1); // give the web browser time to receive the data

    // close the connection:
    client.stop();
    Serial.println("[Client disconnected]");
  }
  delay(1000);
}
