import { Image } from "expo-image";
import {
  StyleSheet,
  ScrollView,
  View,
  TouchableOpacity,
  Text,
  Dimensions,
} from "react-native";
import { database } from "@/firebaseConfig";
import { ref, onValue } from "firebase/database";
import { useEffect, useState } from "react";
import ParallaxScrollView from "@/components/ParallaxScrollView";
import { ThemedText } from "@/components/ThemedText";
import { ThemedView } from "@/components/ThemedView";
import { LineChart } from "react-native-chart-kit";

const screenWidth = Dimensions.get("window").width;

export default function HistoricalDataScreen() {
  const [historicalData, setHistoricalData] = useState<any[]>([]);
  const [selectedView, setSelectedView] = useState<"day" | "hour">("day");
  const [dailyAverage, setDailyAverage] = useState<number[]>([]);
  const [hourlyAverage, setHourlyAverage] = useState<number[]>([]);

  useEffect(() => {
    const historialRef = ref(database, "estacionamientos/historico");

    const unsubscribe = onValue(historialRef, (snapshot) => {
      const data = snapshot.val();
      if (!data) {
        console.warn("No hay datos históricos disponibles.");
        return;
      }

      const formattedData = Object.keys(data).map((key) => ({
        dia: key,
        info: data[key],
      }));

      setHistoricalData(formattedData);
    });

    return () => unsubscribe();
  }, []);

  useEffect(() => {
    if (historicalData.length > 0) {
      const dailyAverages = historicalData.map((day) => {
        const dayAverages = Object.values(day.info).map(
          (hourData: any) => hourData.promedio
        );
        const dayAverage =
          dayAverages.reduce((acc: number, curr: number) => acc + curr, 0) /
          dayAverages.length;
        return dayAverage;
      });
      setDailyAverage(dailyAverages);

      const lastDay = historicalData[historicalData.length - 1];
      const hourlyAverages = Object.values(lastDay.info).map(
        (hourData: any) => hourData.promedio
      );
      setHourlyAverage(hourlyAverages);
    }
  }, [historicalData]);

  const dailyData = {
    labels: historicalData.map((day) => day.dia),
    datasets: [{ data: dailyAverage, strokeWidth: 2 }],
  };

  const hourlyData = {
    labels: Object.keys(historicalData[historicalData.length - 1]?.info || {}),
    datasets: [{ data: hourlyAverage, strokeWidth: 2 }],
  };

  const chartConfig = {
    backgroundColor: "#FFFFFF",
    backgroundGradientFrom: "#FFFFFF",
    backgroundGradientTo: "#FFFFFF",
    decimalPlaces: 2,
    color: (opacity = 1) => `rgba(30, 30, 30, ${opacity})`,
    labelColor: (opacity = 1) => `rgba(30, 30, 30, ${opacity})`,
    strokeWidth: 3,
    barPercentage: 0.7,
    propsForDots: {
      r: "4",
      strokeWidth: "2",
      stroke: "#A1CEDC",
    },
  };

  return (
    <ParallaxScrollView
      headerBackgroundColor={{ light: "#A1CEDC", dark: "#1D3D47" }}
      headerImage={
        <Image
          source={require("@/assets/images/ParkingLot.png")}
          style={styles.parkingLotImage}
        />
      }
    >
      <ThemedView style={styles.titleContainer}>
        <ThemedText type="title">Datos Históricos de Estacionamiento</ThemedText>
      </ThemedView>

      <View style={styles.buttonContainer}>
        <TouchableOpacity
          style={[
            styles.toggleButton,
            selectedView === "day" && styles.activeButton,
          ]}
          onPress={() => setSelectedView("day")}
        >
          <Text style={styles.buttonText}>Promedio por Día</Text>
        </TouchableOpacity>
        <TouchableOpacity
          style={[
            styles.toggleButton,
            selectedView === "hour" && styles.activeButton,
          ]}
          onPress={() => setSelectedView("hour")}
        >
          <Text style={styles.buttonText}>Promedio por Hora</Text>
        </TouchableOpacity>
      </View>

      <ScrollView contentContainerStyle={styles.scrollContainer}>
        <View style={styles.chartContainer}>
          <ThemedText type="subtitle" style={styles.chartTitle}>
            {selectedView === "day"
              ? "Promedio de Ocupación por Día"
              : "Promedio de Ocupación por Hora (Último Día)"}
          </ThemedText>
          <LineChart
            data={selectedView === "day" ? dailyData : hourlyData}
            width={screenWidth - 40}
            height={300}
            chartConfig={chartConfig}
            bezier
            style={styles.chart}
          />
        </View>
      </ScrollView>
    </ParallaxScrollView>
  );
}

const styles = StyleSheet.create({
  titleContainer: {
    flexDirection: "row",
    alignItems: "center",
    marginBottom: 12,
  },
  buttonContainer: {
    flexDirection: "row",
    justifyContent: "space-between",
    marginBottom: 20,
  },
  toggleButton: {
    backgroundColor: "#F0F0F0",
    paddingVertical: 10,
    paddingHorizontal: 20,
    borderRadius: 20,
  },
  activeButton: {
    backgroundColor: "#A1CEDC",
  },
  buttonText: {
    fontWeight: "600",
    color: "#1D3D47",
  },
  chartContainer: {
    backgroundColor: "#FFFFFF",
    borderRadius: 16,
    marginBottom: 30,

  },
  chartTitle: {
    marginBottom: 10,
    fontWeight: "600",
    fontSize: 22,
  },
  chart: {
    borderRadius: 12,
  },
  parkingLotImage: {
    height: 310,
    width: 310,
    top: 50,
    bottom: -90,
    left: -35,
    position: "absolute",
  },
  scrollContainer: {
    paddingBottom: 100,
  },
});
