$(document).ready(function() {
    var mapOptions = {
        center: new google.maps.LatLng(1.357371, 103.829624),
        zoom: 12,
        mapTypeId: google.maps.MapTypeId.ROADMAP
    };
    var map = new google.maps.Map(document.getElementById("map_canvas"), mapOptions);

    var markers = [];

    google.maps.Map.prototype.clearMarkers = function() {
        for (var i = 0; i < markers.length; ++i)
            markers[i].setMap(null);
        markers = [];
    }

    var genResults = function(time, data) {
        $("#search_results").append(
            '<div id="time_show">'+
            'Total '+data.length+' results ('+time.toFixed(6)+' seconds)'+
            '</div>');

        for (var i = 0; i < data.length; ++i) {
            var loc = data[i];
            var ret = '<div class="result_item" id="result_item_'+i+'">';
            ret += '<span class="name">'+loc['name']+'</span><br>';
            ret += '<span class="latlng">'+loc['latlng']+'</span><br>';
            ret += '<span class="addr">'+loc['addr']+'</span>';
            ret += '</div>';
            $("#search_results").append(ret);
        }

        $(".result_item").click(function() {
            var id = $(this).attr("id").substr(12);
            id = parseInt(id);
            markers[id].setAnimation(google.maps.Animation.DROP);
        });
    };

    var newQuery = function(pan) {
        map.clearMarkers();
        $("#search_results").html('');

        var qstr = $("#search_bar").val();
        if (qstr.length < 1) return;

        var url = '/search?';
        url += 'q=' + qstr;
        url += '&lat=' + map.getCenter().lat();
        url += '&lng=' + map.getCenter().lng();

        $.getJSON(url, function(response) {
            var time = response['time'];
            var data = response['result'];

            for (var i = 0; i < data.length; ++i) {
                var loc = data[i];
                var lat = loc['latlng'][0];
                var lng = loc['latlng'][1];
                var marker = new google.maps.Marker({
                    position: new google.maps.LatLng(lat, lng),
                    map: map,
                    title: loc['addr']
                });
                markers.push(marker);
            }

            if (pan && data.length > 0) {
                var lat = data[0]['latlng'][0];
                var lng = data[0]['latlng'][1];
                map.panTo(new google.maps.LatLng(lat, lng));
            }

            genResults(time, data);
        })
    };

    $("#search_bar").on("input", function() { newQuery(true); });
    google.maps.event.addListener(map, 'dragend', function() { newQuery(false); });
    google.maps.event.addListener(map, 'zoom_changed', function() { newQuery(false); });
});
