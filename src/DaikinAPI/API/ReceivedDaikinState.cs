namespace DaikinAPI.API
{
    public class ReceivedDaikinState
    {
        [System.Text.Json.Serialization.JsonPropertyName("timestamp")]
        public int Timestamp { get; set; }

        [System.Text.Json.Serialization.JsonPropertyName("temperature")]
        public int Temperature { get; set; }

        [System.Text.Json.Serialization.JsonPropertyName("fan")]
        public int Fan { get; set; }

        [System.Text.Json.Serialization.JsonPropertyName("powerState")]
        public int PowerState { get; set; }

        [System.Text.Json.Serialization.JsonPropertyName("swing")]
        public int Swing { get; set; }

        [System.Text.Json.Serialization.JsonPropertyName("timerOn")]
        public int TimerOn { get; set; }

        [System.Text.Json.Serialization.JsonPropertyName("timerOnValue")]
        public int TimerOnValue { get; set; }

        [System.Text.Json.Serialization.JsonPropertyName("timerOff")]
        public int TimerOff { get; set; }

        [System.Text.Json.Serialization.JsonPropertyName("timerOffValue")]
        public int TimerOffValue { get; set; }

        [System.Text.Json.Serialization.JsonPropertyName("mode")]
        public int Mode { get; set; }
    }
}
