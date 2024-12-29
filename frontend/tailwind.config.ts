import typography from '@tailwindcss/typography';
import flowbite from 'flowbite/plugin';
import type { Config } from 'tailwindcss';

export default {
  content: [
    './src/**/*.{html,js,svelte,ts}',
    './node_modules/flowbite-svelte/**/*.{html,js,svelte,ts}"',
    "./node_modules/flowbite-svelte-icons/**/*.{html,js,svelte,ts}"
  ],
  darkMode: 'class',
  theme: {
    extend: {
      colors: {
        // flowbite-svelte
        // blue
        primary: {"50":"#eff6ff","100":"#dbeafe","200":"#bfdbfe","300":"#93c5fd","400":"#60a5fa","500":"#3b82f6","600":"#2563eb","700":"#1d4ed8","800":"#1e40af","900":"#1e3a8a"}
      }
    }
  },
  plugins: [
    typography,
    flowbite
  ]
} satisfies Config;
