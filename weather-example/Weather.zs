struct Temperature
{
    int32 value;
};

struct City
{
    string name;
};

service CurrentTemperatureService
{
    Temperature getTemperatureIn(City);
};

struct WeatherDescription {
    string description;
};

pubsub WeatherTopics
{
    topic("weather/current") WeatherDescription currentWeather;
};
