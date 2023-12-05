import 'dart:collection';

import 'package:flutter/material.dart';
import 'package:dio/dio.dart';
import 'dart:developer';
import 'dart:async';
import 'dart:collection';

import 'package:search_choices/search_choices.dart';
import 'package:weather_icons/weather_icons.dart';
import 'package:flutter_staggered_animations/flutter_staggered_animations.dart';

part 'search.dart';
part 'details.dart';
part 'place_details.dart';

const PLACES_API_KEY = "25789e38-d2b5-42a0-8a9f-8213cdccd031";
const WEATHER_API_KEY = "a740911c16ddd604e7cecf5bc27ad430";
const TRIPS_API_KEY =
    "5ae2e3f221c38a28845f05b6163f3e2689b5f345ae4f0454d4c0892b";

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'mr white why are we in the window title xDDDDDDDDDD',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: const MyHomePage(
          title: 'mr white why are we in the header xDDDDDDDDDD'),
    );
  }
}

class Place {
  final String place;
  final List<double> coords;
  final String? city;
  final String? postcode;

  bool hasDetails = false;
  double? temp;
  int wid = -1; // cant be null cuz of getters... bruh.
  String weatherName = "";

  Place(this.place, this.city, this.postcode, this.coords);

  String toName() {
    String out = "";
    if (city != null) out += "${city!}, ";
    if (postcode != null) out += "${postcode!}, ";
    out += place;

    return out;
  }

  static Place fromGHReply(Map<String, dynamic> map) {
    Place ret = Place(map["name"], map["city"], map["postcode"],
        [map["point"]["lat"], map["point"]["lng"]]);

    return ret;
  }

  double get latitude {
    return coords[0];
  }

  double get longitude {
    return coords[1];
  }

  set weatherID(int id) => wid = id;
  int get weatherID => wid;
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage>
    with SingleTickerProviderStateMixin {
  Place? currentPlace;
  List<TripPlace>? _fts;

  late final AnimationController _ancont = AnimationController(
    duration: const Duration(seconds: 2),
    vsync: this,
  );

  void _selectLocation(Place place) async {
    setState(() {
      currentPlace = place;
      print(
          "Selected ${place.toName()} @ ${place.latitude}, ${place.longitude}");
    });

    var wfut = Dio().get(
      "https://api.openweathermap.org/data/2.5/weather",
      queryParameters: {
        "lat": place.latitude,
        "lon": place.longitude,
        "appid": WEATHER_API_KEY,
      },
    );

    // weather
    wfut.then((value) {
      var info = value.data;

      var what = info["weather"][0];

      place.weatherID = what["id"];
      place.temp = info["main"]["temp"];
      place.weatherName = capitalizeEachWord(what["description"]);

      setState(() {});
    });

    var tfut = Dio().get(
      "http://api.opentripmap.com/0.1/en/places/radius",
      queryParameters: {
        "lat": place.latitude,
        "lon": place.longitude,
        "radius": 1500,
        "lang": "en",
        "limit": 50,
        "apikey": TRIPS_API_KEY,
      },
    );

    tfut.then((value) {
      if (place != currentPlace) {
        return; // too late to the party
      }

      var info = value.data;

      var fts = info["features"];
      _setFeatures(fts);
      setState(() {});
    });
  }

  Timer? _debounce;

  Future<List<Place>> fetchPlaces(filter, int? pageNb) async {
    if (filter == "") return [];

    var response = await Dio().get(
      "https://graphhopper.com/api/1/geocode",
      queryParameters: {
        "q": filter,
        "locale": "en",
        "key": PLACES_API_KEY,
        "limit": 15
      },
    );

    final data = response.data;
    if (data != null) {
      List<Place> out = [
        for (var entry in data["hits"]) Place.fromGHReply(entry)
      ];

      return out;
    }

    return [];
  }

  PointerThisPlease<int> curPage = PointerThisPlease<int>(1);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(widget.title),
      ),
      body: Center(
        child: Column(
          children: <Widget>[
            _buildSearch(context),
            ...?_buildDetails(context),
          ],
        ),
      ),
    );
  }
}
