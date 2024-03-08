#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>EyeTracker</title>
  <script type="text/javascript" src="http://localhost:8080/eyeTracker.js"></script>
</head>
<body>
<div style="width: 100%; height: 100%" id="body"></div>
</body>
</html>
)rawliteral";