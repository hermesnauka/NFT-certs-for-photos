'use client';

import Link from 'next/link';
import { CertificateDto } from '../lib/types';
import { ipfsToGatewayUrl } from '../lib/api';

export function CertificateCard({ certificate }: { certificate: CertificateDto }) {
  return (
    <Link
      href={`/certificate/${certificate.tokenId}`}
      className="group block overflow-hidden rounded-lg border border-ink/10 bg-white shadow-sm transition hover:shadow-md"
    >
      <div className="aspect-square w-full overflow-hidden bg-ink/5">
        {/* eslint-disable-next-line @next/next/no-img-element */}
        <img
          src={ipfsToGatewayUrl(certificate.imageIpfsUri)}
          alt={certificate.title}
          className="h-full w-full object-cover transition group-hover:scale-105"
        />
      </div>
      <div className="space-y-1 p-4">
        <p className="truncate font-serif text-base font-semibold">{certificate.title}</p>
        <p className="text-xs text-ink/60">Token #{certificate.tokenId}</p>
        <p className="text-xs text-ink/60">
          Royalty: {(certificate.royaltyPercentageBps / 100).toFixed(2)}%
        </p>
      </div>
    </Link>
  );
}
