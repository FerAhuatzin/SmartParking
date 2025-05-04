import { Tabs } from 'expo-router';
import React from 'react';
import { Platform } from 'react-native';
import {HomeIcon, AnalyticsIcon} from '@/constants/icons'; // Assuming you have these icons available
import { HapticTab } from '@/components/HapticTab';
import TabBarBackground from '@/components/ui/TabBarBackground';
import { Colors } from '@/constants/Colors';
import { useColorScheme } from '@/hooks/useColorScheme';

export default function TabLayout() {
  const colorScheme = useColorScheme();

  return (
    <Tabs
      screenOptions={{
        tabBarActiveTintColor: Colors[colorScheme ?? 'light'].tint,
        headerShown: false,
        tabBarButton: HapticTab,
        tabBarBackground: TabBarBackground,
        tabBarStyle: Platform.select({
          ios: {
            // Use a transparent background on iOS to show the blur effect
            position: 'absolute',
          },
          default: {},
        }),
      }}>
      <Tabs.Screen
        name="index"
        options={{
          title: 'Home',
          tabBarIcon: ({ color }) => <HomeIcon size={28} color={color} />,
        }}
      />
      <Tabs.Screen
        name="historic-data"
        options={{
          title: 'Analytics',
          tabBarIcon: ({ color }) => <AnalyticsIcon size={28} color={color} />,
        }}
      />
    </Tabs>
  );
}
