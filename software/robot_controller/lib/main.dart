
import 'dart:math';

import 'package:control_pad/models/pad_button_item.dart';
import 'package:flutter/material.dart';
import 'package:control_pad/control_pad.dart';
import 'package:control_pad/models/gestures.dart';

import 'package:web_socket_channel/io.dart';
import 'package:web_socket_channel/web_socket_channel.dart';
//import 'package:web_socket_channel/html.dart';

void main() {
  runApp(ExampleApp());
}
   
class ExampleApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      
      title: 'Nawak Controller',
      home: HomePage(),
    );
  }
}

class HomePage extends StatefulWidget {
  @override
  _HomePageState createState() => _HomePageState();
  
  
}


class _HomePageState extends State<HomePage> {
  WebSocketChannel channel;
  TextEditingController controller;
  var lastTime = new DateTime.now();

  @override
  void initState() {
    super.initState();
    channel = IOWebSocketChannel.connect('ws://192.168.1.29');//connect('ws://echo.websocket.org');//HtmlWebSocketChannel.connect("ws://192.168.1.29");
    controller = TextEditingController();

  }

  void sendData(int mx, int my, bool force) {
      var now = new DateTime.now();
      var earlier = now.subtract(new Duration(milliseconds: 250));
      print("force : $force");
      if (force || lastTime.isBefore(earlier)) {
        print("X : $mx, Y : $my, lastTime : $lastTime now: $now, earlier: $earlier");
        lastTime = now;
        
        var s = mx.toString() + " " + my.toString();
        channel.sink.add(s);
      }  
  }
  @override
  void dispose() {
    channel.sink.close();
    super.dispose();
  }

  JoystickDirectionCallback onDirectionChanged(
      double degrees, double distance) {
      // print(
      //   "Degree : ${degrees.toStringAsFixed(2)}, Distance : ${distance.toStringAsFixed(2)}  ");
      double y = (distance * sin(degrees * pi / 180)) * 100 * distance;
      double x = (distance * cos(degrees * pi / 180)) * 100 * distance;

      sendData(x.toInt(), y.toInt(), (x == 0 && y == 0));
      

  }

  PadButtonPressedCallback padButtonPressedCallback(
    int buttonIndex, Gestures gesture){
    print("ButtonIndex : $buttonIndex");

  }

  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        backgroundColor: Color.fromARGB(0xff, 0xc8, 0x5a, 0x54),
        title: Text('Nawak Controller'),
      ),
      body: Container(
        color: Color.fromARGB(0xff, 0xff, 0x8a, 0x80),
        child: Column(
          children: <Widget>[
            Form(
              child: TextFormField(
                  controller: controller,
                  decoration: InputDecoration(
                    labelText: "Send to websocket",
                  ),
              ),
            ),
            StreamBuilder(
              stream: channel.stream,
              builder: (BuildContext context, AsyncSnapshot snapshot) {
                return Container(
                  child: Text(snapshot.hasData ? '${snapshot.data}' : '')
                );
              },
            ),
            Row(
                mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                children: <Widget>[
                  JoystickView(
                    iconsColor: Colors.black,
                    innerCircleColor: Color.fromARGB(0xff, 0xff, 0x40, 0x81),
                    backgroundColor: Color.fromARGB(0xff, 0xff, 0xbc, 0xaf),
                    onDirectionChanged: onDirectionChanged,
                  ),
                  PadButtonsView(
                    buttons: const [
                      PadButtonItem(index: 0, backgroundColor: Colors.red, pressedColor: Colors.grey, buttonText: "A"),
                      PadButtonItem(index: 1, backgroundColor: Colors.yellow, pressedColor: Colors.grey, buttonText: "B"),
                      PadButtonItem(index: 2, backgroundColor: Colors.green, pressedColor: Colors.grey, buttonText: "X"),
                      PadButtonItem(index: 3, backgroundColor: Colors.blue, pressedColor: Colors.grey, buttonText: "Y")
                    ],
                    padButtonPressedCallback: padButtonPressedCallback,
                  ),
                ],
              ),
          ]
        ),
      ),
      floatingActionButton: FloatingActionButton (
        child: Icon(Icons.send),
        onPressed: () {
          sendData(0, 0, true);
        },
        ),
    );
  }
}

