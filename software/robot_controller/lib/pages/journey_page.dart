
import 'package:flutter/material.dart';

class JourneyPage extends StatefulWidget {
  JourneyPage({Key key, this.title}) : super(key: key);
  final String title;

  @override
  _JourneyPageState createState() => _JourneyPageState();
}

class _JourneyPageState extends State<JourneyPage> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(widget.title),
      ),
      body: GridView.count(
          // Create a grid with 2 columns. If you change the scrollDirection to
          // horizontal, this produces 2 rows.
          crossAxisCount: 2,
          // Generate 100 widgets that display their index in the List.
          children: List.generate(6, (index) {
            return Container(
              margin: EdgeInsets.all(32),
              decoration: BoxDecoration(
                color: Colors.amber,
                shape: BoxShape.rectangle,
                borderRadius: BorderRadius.circular(8.0),
                boxShadow: <BoxShadow>[
                  BoxShadow(  
                    color: Colors.black12,
                    blurRadius: 10.0,
                    offset: Offset(0.0, 10.0),
                  ),
                ],
              ),
              child: Center(
                child: Text(
                  'Niveau ${index+1}',
                  style: Theme.of(context).textTheme.headline5,
                ),
              ),
            );
          }),
        ),
      floatingActionButton: FloatingActionButton(
        tooltip: 'Increment',
        onPressed: () { Navigator.pushNamed(context, '/'); },
        child: Icon(Icons.add),
      ),
    );
  }
}