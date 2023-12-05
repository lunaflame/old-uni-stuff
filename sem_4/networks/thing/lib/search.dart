// ignore_for_file: library_private_types_in_public_api

part of 'main.dart';

extension Search on _MyHomePageState {
  _buildSearch(BuildContext context) {
    return SearchChoices.single(
      searchHint: "Search place...",
      value: currentPlace,

      /* graphhopper doesn't support pagination so this is pointless */

      //itemsPerPage: 10,
      //currentPage: curPage,

      isExpanded: true,
      futureSearchFn: (String? keyword, String? orderBy, bool? orderAsc,
          List<Tuple2<String, String>>? filters, int? pageNb) async {
        if (_debounce?.isActive ?? false) _debounce!.cancel();

        final completer = Completer(); // bro wtf is this

        _debounce = Timer(const Duration(milliseconds: 300), () async {
          List<dynamic> dataOut = await fetchPlaces(keyword, pageNb);

          List<DropdownMenuItem> results = [
            for (var item in dataOut)
              DropdownMenuItem(
                value: item,
                child: Card(
                  shape: RoundedRectangleBorder(
                    borderRadius: BorderRadius.circular(4),
                    side: const BorderSide(
                      color: Colors.blue,
                      width: 1,
                    ),
                  ),
                  child: Padding(
                    padding: const EdgeInsets.all(4),
                    child: Text(item.toName()),
                  ),
                ),
              ),
          ];

          Tuple2<List<DropdownMenuItem>, int> ret =
              Tuple2<List<DropdownMenuItem>, int>(results, results.length);
          completer.complete(ret);
        });

        return await completer.future;
      },
      selectedValueWidgetFn: (Place item) {
        return (Padding(
          padding: const EdgeInsets.all(6),
          child: FittedBox(
            fit: BoxFit.fitWidth,
            child: Text(item.toName()),
          ),
        ));
      },
      dialogBox: false,
      menuConstraints: BoxConstraints.tight(const Size.fromHeight(450)),

      // asyncItems: (filter) => _createSearchItems(filter),
      /*dropdownDecoratorProps: const DropDownDecoratorProps(
          dropdownSearchDecoration: InputDecoration(
            labelText: "Location",
            hintText: "whatever",
            border: OutlineInputBorder(),
          ),
        ),*/
      onChanged: _selectLocation,
      onClear: () {
        currentPlace = null;
        _resetFeatures();
        setState(() {});
      },
    );
  }
}
