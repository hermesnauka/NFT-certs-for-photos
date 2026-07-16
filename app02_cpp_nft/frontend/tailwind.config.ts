import type { Config } from 'tailwindcss';

const config: Config = {
  content: ['./app/**/*.{ts,tsx}', './components/**/*.{ts,tsx}'],
  theme: {
    extend: {
      colors: {
        ink: '#14181f',
        parchment: '#f7f4ee',
        brass: '#b8863b',
      },
    },
  },
  plugins: [],
};

export default config;
