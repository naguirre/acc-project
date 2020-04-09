import 'package:flutter/material.dart';
import 'package:roboto_controller/pages/home_page.dart';
import 'package:roboto_controller/pages/journey_page.dart';


class Application extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Robot Controller',
      theme: ThemeData(
        primarySwatch: Colors.pink,
        visualDensity: VisualDensity.adaptivePlatformDensity,
      ),
      initialRoute: '/',
      routes: {
        // When navigating to the "/" route, build the FirstScreen widget.
        '/': (context) => HomePage(title: "Main menu"),
        // When navigating to the "/second" route, build the SecondScreen widget.
        '/journey': (context) => JourneyPage(title: "Journey",),
      },
    );
  }
}


