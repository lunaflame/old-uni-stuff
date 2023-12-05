part of 'main.dart';

/*
enum Weather {
  cloudy("Cloudy", "cloudy"),
  clear("Clear Sky", "clear"),
  rain("Raining", "rain"),
  unknown("Unknown?", ""),
  ;

  const Weather(this.name, this.id);

  static Weather fromID(String id) {
    // O(n); cry about it lol
    return values.firstWhere((element) => element.id == id);
  }

  final String name;
  final String id;
}*/

const Map<String, IconData> iconConv = {
  'wi-owm-200': WeatherIcons.thunderstorm,
  'wi-owm-201': WeatherIcons.thunderstorm,
  'wi-owm-202': WeatherIcons.thunderstorm,
  'wi-owm-210': WeatherIcons.lightning,
  'wi-owm-211': WeatherIcons.lightning,
  'wi-owm-212': WeatherIcons.lightning,
  'wi-owm-221': WeatherIcons.lightning,
  'wi-owm-230': WeatherIcons.thunderstorm,
  'wi-owm-231': WeatherIcons.thunderstorm,
  'wi-owm-232': WeatherIcons.thunderstorm,
  'wi-owm-300': WeatherIcons.sprinkle,
  'wi-owm-301': WeatherIcons.sprinkle,
  'wi-owm-302': WeatherIcons.rain,
  'wi-owm-310': WeatherIcons.rain_mix,
  'wi-owm-311': WeatherIcons.rain,
  'wi-owm-312': WeatherIcons.rain,
  'wi-owm-313': WeatherIcons.showers,
  'wi-owm-314': WeatherIcons.rain,
  'wi-owm-321': WeatherIcons.sprinkle,
  'wi-owm-500': WeatherIcons.sprinkle,
  'wi-owm-501': WeatherIcons.rain,
  'wi-owm-502': WeatherIcons.rain,
  'wi-owm-503': WeatherIcons.rain,
  'wi-owm-504': WeatherIcons.rain,
  'wi-owm-511': WeatherIcons.rain_mix,
  'wi-owm-520': WeatherIcons.showers,
  'wi-owm-521': WeatherIcons.showers,
  'wi-owm-522': WeatherIcons.showers,
  'wi-owm-531': WeatherIcons.storm_showers,
  'wi-owm-600': WeatherIcons.snow,
  'wi-owm-601': WeatherIcons.snow,
  'wi-owm-602': WeatherIcons.sleet,
  'wi-owm-611': WeatherIcons.rain_mix,
  'wi-owm-612': WeatherIcons.rain_mix,
  'wi-owm-615': WeatherIcons.rain_mix,
  'wi-owm-616': WeatherIcons.rain_mix,
  'wi-owm-620': WeatherIcons.rain_mix,
  'wi-owm-621': WeatherIcons.snow,
  'wi-owm-622': WeatherIcons.snow,
  'wi-owm-701': WeatherIcons.showers,
  'wi-owm-711': WeatherIcons.smoke,
  'wi-owm-721': WeatherIcons.day_haze,
  'wi-owm-731': WeatherIcons.dust,
  'wi-owm-741': WeatherIcons.fog,
  'wi-owm-761': WeatherIcons.dust,
  'wi-owm-762': WeatherIcons.dust,
  'wi-owm-771': WeatherIcons.cloudy_gusts,
  'wi-owm-781': WeatherIcons.tornado,
  'wi-owm-800': WeatherIcons.day_sunny,
  'wi-owm-801': WeatherIcons.cloudy_gusts,
  'wi-owm-802': WeatherIcons.cloudy_gusts,
  'wi-owm-803': WeatherIcons.cloudy_gusts,
  'wi-owm-804': WeatherIcons.cloudy,
  'wi-owm-900': WeatherIcons.tornado,
  'wi-owm-901': WeatherIcons.storm_showers,
  'wi-owm-902': WeatherIcons.hurricane,
  'wi-owm-903': WeatherIcons.snowflake_cold,
  'wi-owm-904': WeatherIcons.hot,
  'wi-owm-905': WeatherIcons.windy,
  'wi-owm-906': WeatherIcons.hail,
  'wi-owm-957': WeatherIcons.strong_wind,
};

final fetchQueue = Queue<TripPlace>();

const int concurrentRequests = 1;
const int requestDelay = 200; // in ms

// on fail, try to fetch 2 more times... if we still fail; abort
const int fetchTries = 3;

int currentRequests = 0;

enum Status {
  queued(""),
  fetching("<fetching...>"),
  failed("<failed>"),
  success("lolwut");

  const Status(this.name);
  final String name;
}

class TripPlace {
  final String xid;
  final String name;
  String? desc;
  Status status = Status.queued;
  int _triedFetch = 0;

  Completer? _fetchComp;
  Future? onFetch;

  TripPlace(this.xid, this.name);

  static void _queueNext() {
    currentRequests--;
    if (fetchQueue.isNotEmpty) {
      fetchQueue.removeFirst().fetch();
    }
  }

  void _doFetch() async {
    var cur = DateTime.now();
    _triedFetch++;

    try {
      var resp = await Dio().get(
        "http://api.opentripmap.com/0.1/en/places/xid/$xid",
        queryParameters: {
          "xid": xid,
          "lang": "en",
          "apikey": TRIPS_API_KEY,
        },
      );

      var passed = DateTime.now().difference(cur).inMilliseconds;
      var toWait = requestDelay - passed;

      if (toWait <= 0) {
        _queueNext();
      } else {
        Future.delayed(Duration(milliseconds: toWait), _queueNext);
      }

      var dat = resp.data;
      if (dat["wikipedia_extracts"] != null) {
        desc = dat["wikipedia_extracts"]!["text"];
      }
      status = Status.success;
      _fetchComp!.complete(dat);
    } catch (e) {
      var passed = DateTime.now().difference(cur).inMilliseconds;
      var toWait = requestDelay - passed;

      if (_triedFetch < fetchTries) {
        Future.delayed(Duration(milliseconds: toWait),
            _doFetch); // try fetching a few times more
      } else {
        // give up.
        print("Failed to fetch place trivia $fetchTries times in a row...");
        status = Status.failed;
        _queueNext();
      }
    }
  }

  Future fetch() async {
    if (onFetch == null) {
      _fetchComp = Completer();
      onFetch = _fetchComp!.future;
    }

    if (currentRequests >= concurrentRequests) {
      fetchQueue.add(this);
      return onFetch;
    }

    currentRequests++;
    _doFetch();

    return onFetch;
  }
}

String capitalizeEachWord(String src) {
  return src.replaceAll(RegExp(' +'), ' ').split(" ").map((str) {
    return str.isNotEmpty ? '${str[0].toUpperCase()}${str.substring(1)}' : '';
  }).join(" ");
}

extension DetailsPage on _MyHomePageState {
  _buildDetails(BuildContext context) {
    if (currentPlace == null) {
      return;
    }

    Place pl = currentPlace!;
    if (pl.weatherID == -1) {
      return const [
        SizedBox(height: 8),
        Center(
          child: SizedBox(
            height: 32,
            width: 32,
            child: CircularProgressIndicator(),
          ),
        ),
      ];
    }
    List<Widget> out = List.empty(growable: true);

    IconData? ic = iconConv["wi-owm-${pl.weatherID}"];

    // build weather
    out.add(
      AnimationLimiter(
        child: Column(
          children: AnimationConfiguration.toStaggeredList(
            childAnimationBuilder: (widget) => SlideAnimation(
              verticalOffset: -16,
              child: FadeInAnimation(child: widget),
            ),
            children: [
              Center(
                child: Row(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    BoxedIcon(
                      ic ?? WeatherIcons.alien,
                    ),
                    SizedBox(width: 8),
                    Text(
                      "${pl.weatherName}, ${(pl.temp! - 273.15).toStringAsFixed(1)}C",
                      textScaleFactor: 1.5,
                      style: const TextStyle(
                        fontFamily: 'Nunito',
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                  ],
                ),
              ),
            ],
          ),
        ),
      ),
    );

    out.add(_buildFeatures(context));

    return out;
  }

  Widget _buildFeatures(BuildContext context) {
    if (_fts == null) return SizedBox();

    List<Widget> out = [
      SizedBox(height: 8),
      const Center(
        child: Text(
          "Nearby places to visit...",
          textScaleFactor: 1.75,
          style: TextStyle(
            fontWeight: FontWeight.bold,
            fontFamily: "Noto Sans",
          ),
        ),
      ),
      SizedBox(height: 8),
    ];

    _fts!.forEach((dat) {
      if (dat.name.length < 2) {
        // so this is a thing, somehow
        return;
      }

      List<Widget> entryElems = [
        Text(
          dat.name,
          textScaleFactor: 1.25,
          textAlign: TextAlign.left,
          style: const TextStyle(
            fontWeight: FontWeight.bold,
            fontFamily: 'Nunito',
          ),
        )
      ];

      bool shouldFade = (dat.desc == null) || (dat.status != Status.success);

      Color clr = dat.status == Status.failed
          ? Colors.red.shade800
          : dat.status == Status.success
              ? Colors.grey.shade700
              : shouldFade
                  ? Color.fromRGBO(80, 80, 80, 0.5)
                  : Colors.black;

      String desc = dat.status == Status.success
          ? (dat.desc ?? "<no description>")
          : dat.status.name;

      bool done = dat.status == Status.success || dat.status == Status.failed;

      /*entryElems.addAll(
        AnimationConfiguration.toStaggeredList(
          childAnimationBuilder: (widget) {
            return SlideAnimation(
              verticalOffset: -8,
              child: FadeInAnimation(child: widget),
            );
          },
          children: [
            Padding(
              padding: EdgeInsets.fromLTRB(8, 0, 0, 0),
              child: Text(
                desc,
                maxLines: 3,
                style: TextStyle(
                  color: clr,
                  fontFamily: 'Nunito',
                ),
              ),
            )
          ],
        ),
      );*/
      entryElems.add(Padding(
        padding: EdgeInsets.fromLTRB(8, 0, 0, 0),
        child: Text(
          desc,
          maxLines: 3,
          style: TextStyle(
            color: clr,
            fontFamily: 'Nunito',
          ),
        ),
      ));

      entryElems.add(SizedBox(height: 8));

      var _offsetAnimation = Tween<Offset>(
        begin: const Offset(0, -1),
        end: const Offset(0, 0),
      ).animate(CurvedAnimation(
        parent: _ancont,
        curve: Curves.easeInCubic,
      ));

      out.add(
        ExpandedSection(
          expand: done,
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: entryElems,
          ),
        ),
      );
    });

    return Flexible(
      child: ListView(
        shrinkWrap: true,
        scrollDirection: Axis.vertical,
        children: out,
        addAutomaticKeepAlives: true,
      ),
    );
  }

  _setFeatures(List<dynamic> fts) {
    fetchQueue.clear(); // no longer need any of the old ones, get fucked
    _fts = [];

    fts.forEach((val) {
      var trip = TripPlace(val["properties"]["xid"], val["properties"]["name"]);
      _fts!.add(trip);

      trip.fetch().then((value) {
        setState(() {});
      });
    });
  }

  _resetFeatures() {
    _fts = null;
    fetchQueue.clear();
  }
}
