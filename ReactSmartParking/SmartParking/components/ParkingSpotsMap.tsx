// components/ParkingSpotsMap.tsx
import React from "react";
import { View, Text, StyleSheet } from "react-native";

type SpotStatus = "available" | "occupied";

interface ParkingSpotsMapProps {
  spots: SpotStatus[];
}

export default function ParkingSpotsMap({ spots }: ParkingSpotsMapProps) {
  return (
    <View style={styles.container}>
      {spots.map((status, index) => (
        <View
          key={index}
          style={[
            styles.spot,
            status === "available" ? styles.available : styles.occupied,
          ]}
        >
          <Text style={styles.label}>
            {status === "available" ? "Libre" : "Ocupado"}
          </Text>
        </View>
      ))}
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flexDirection: "row",
    justifyContent: "space-between",
    paddingHorizontal: 20,
    marginTop: 16,
  },
  spot: {
    width: 80,
    height: 120,
    borderRadius: 10,
    justifyContent: "center",
    alignItems: "center",
    marginHorizontal: 5,
    shadowColor: "#000",
    shadowOpacity: 0.1,
    shadowOffset: { width: 0, height: 2 },
    shadowRadius: 4,
    elevation: 3,
  },
  available: {
    backgroundColor: "#A8E6CF",
  },
  occupied: {
    backgroundColor: "#FF8A80",
  },
  label: {
    color: "#333",
    fontWeight: "bold",
  },
});
