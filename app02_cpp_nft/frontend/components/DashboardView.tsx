'use client';

import { useQuery } from '@tanstack/react-query';
import { useAccount } from 'wagmi';
import { getArtistDashboard } from '../lib/api';
import { useTranslation } from '../lib/i18n/context';
import { CertificateCard } from './CertificateCard';
import { ConnectWalletButton } from './ConnectWalletButton';

export function DashboardView() {
  const { address, isConnected } = useAccount();
  const { t } = useTranslation();

  const { data, isLoading, isError } = useQuery({
    queryKey: ['artist-dashboard', address],
    queryFn: () => getArtistDashboard(address as string),
    enabled: isConnected && !!address,
  });

  if (!isConnected) {
    return (
      <div className="rounded-lg border border-dashed border-ink/20 p-12 text-center">
        <p className="mb-4 text-ink/70">Connect your wallet to view your certificates.</p>
        <div className="flex justify-center">
          <ConnectWalletButton />
        </div>
      </div>
    );
  }

  if (isLoading) {
    return <p className="text-ink/60">Loading your certificates…</p>;
  }

  if (isError) {
    return <p className="text-red-700">Could not load your certificates. Please try again.</p>;
  }

  if (!data || data.totalCertificates === 0) {
    return <p className="text-ink/60">You haven&apos;t minted any certificates yet.</p>;
  }

  return (
    <div className="space-y-6">
      <p className="text-sm text-ink/60">
        {t('dashboard.totalCertificates', 'Total Certificates')}: {data.totalCertificates}
      </p>
      <div className="grid grid-cols-1 gap-6 sm:grid-cols-2 lg:grid-cols-3">
        {data.certificates.map((certificate) => (
          <CertificateCard key={certificate.tokenId} certificate={certificate} />
        ))}
      </div>
    </div>
  );
}
