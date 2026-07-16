import type { Metadata } from 'next';
import { Providers } from '../components/Providers';
import { Header } from '../components/Header';
import './globals.css';

export const metadata: Metadata = {
  title: 'NFT Certs for Photos',
  description: 'Cryptographic certificates of authenticity for fine-art photography.',
};

export default function RootLayout({ children }: { children: React.ReactNode }) {
  return (
    <html lang="en">
      <body>
        <Providers>
          <Header />
          <main className="mx-auto max-w-5xl px-4 py-10">{children}</main>
        </Providers>
      </body>
    </html>
  );
}
