'use client';

import Link from 'next/link';
import { usePathname } from 'next/navigation';
import { ConnectWalletButton } from './ConnectWalletButton';
import { LanguageSwitcher } from './LanguageSwitcher';

export function Header() {
  const pathname = usePathname();

  const navLink = (href: string, label: string) => (
    <Link
      href={href}
      className={`text-sm font-medium ${
        pathname === href ? 'text-ink' : 'text-ink/60 hover:text-ink'
      }`}
    >
      {label}
    </Link>
  );

  return (
    <header className="border-b border-ink/10 bg-parchment/80 backdrop-blur">
      <div className="mx-auto flex max-w-5xl items-center justify-between px-4 py-4">
        <div className="flex items-center gap-8">
          <Link href="/" className="text-lg font-serif font-semibold tracking-tight">
            NFT Certs for Photos
          </Link>
          <nav className="flex items-center gap-6">
            {navLink('/', 'Dashboard')}
            {navLink('/mint', 'Mint')}
          </nav>
        </div>
        <div className="flex items-center gap-4">
          <LanguageSwitcher />
          <ConnectWalletButton />
        </div>
      </div>
    </header>
  );
}
