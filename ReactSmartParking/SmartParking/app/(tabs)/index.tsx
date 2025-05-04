import { Image } from "expo-image";
import { StyleSheet } from "react-native";
import ParallaxScrollView from "@/components/ParallaxScrollView";
import ParkingSpotsMap from "@/components/ParkingSpotsMap";
import { ThemedText } from "@/components/ThemedText";
import { ThemedView } from "@/components/ThemedView";
import { database } from "@/firebaseConfig";
import { ref, onValue } from "firebase/database";
import { useEffect, useState } from "react";

export default function HomeScreen() {
  const [spotStatuses, setSpotStatuses] = useState<
    ("available" | "occupied")[]
  >([]);
  const [availableSpots, setAvailableSpots] = useState(0);
  const [occupiedSpots, setOccupiedSpots] = useState(0);

  useEffect(() => {
    const estacionamientosRef = ref(database, "estacionamientos");
  
    const unsubscribe = onValue(estacionamientosRef, (snapshot) => {
      const data = snapshot.val();
  
      if (!data) {
        console.warn("No hay datos en 'estacionamientos'");
        return;
      }
  
      const { espacios, disponibles, ocupados } = data;
  
      console.log("Espacios:", espacios);
      console.log("Disponibles:", disponibles);
      console.log("Ocupados:", ocupados);
  
      if (espacios) {
        const statuses = Object.values(espacios).map((espacio: any, index) => {
          console.log(`Espacio ${index + 1}:`, espacio);
          return espacio.ocupado ? "occupied" : "available";
        });
        setSpotStatuses(statuses);
      } else {
        console.warn("espacios' no está definido");
      }
  
      setAvailableSpots(disponibles || 0);
      setOccupiedSpots(ocupados || 0);
    });
  
    return () => unsubscribe();
  }, []);
  

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
        <ThemedText type="title">Estacionamiento Isai Serrano</ThemedText>
      </ThemedView>
      <ThemedView style={styles.stepContainer}>
        <ThemedText type="subtitle">
          Cajones disponibles{" "}
          <ThemedText type="highlight_positive">{availableSpots}</ThemedText>
        </ThemedText>
      </ThemedView>
      <ThemedView style={styles.stepContainer}>
        <ThemedText type="subtitle">
          Cajones ocupados{" "}
          <ThemedText type="highlight_negative">{occupiedSpots}</ThemedText>
        </ThemedText>
      </ThemedView>
      <ThemedView style={styles.stepContainer}>
        <ThemedText type="subtitle">Mapa</ThemedText>
        <ParkingSpotsMap spots={spotStatuses} />
      </ThemedView>
    </ParallaxScrollView>
  );
}

const styles = StyleSheet.create({
  titleContainer: {
    flexDirection: "row",
    alignItems: "center",
    gap: 8,
  },
  stepContainer: {
    gap: 8,
    marginBottom: 8,
  },
  parkingLotImage: {
    height: 310,
    width: 310,
    top: 50,
    bottom: -90,
    left: -35,
    position: "absolute",
  },
});
