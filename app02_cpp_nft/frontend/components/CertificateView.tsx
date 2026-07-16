'use client';

import { certificatePdfUrl, ipfsToGatewayUrl } from '../lib/api';
import { CertificateDto } from '../lib/types';
import { useTranslation } from '../lib/i18n/context';

function LinkRow({ label, href, mono }: { label: string; href: string; mono?: boolean }) {
  return (
    <div className="flex items-center justify-between gap-4 border-b border-ink/10 py-3 last:border-0">
      <span className="text-sm text-ink/50">{label}</span>
      <a
        href={href}
        target="_blank"
        rel="noreferrer"
        className={`truncate text-right text-sm text-brass hover:underline ${mono ? 'font-mono text-xs' : ''}`}
      >
        {href}
      </a>
    </div>
  );
}

export function CertificateView({ data }: { data: CertificateDto }) {
  const { t } = useTranslation();

  return (
    <div className="space-y-8">
      <div className="grid gap-8 md:grid-cols-2">
        <div className="overflow-hidden rounded-lg border border-ink/10 bg-white">
          {/* eslint-disable-next-line @next/next/no-img-element */}
          <img
            src={ipfsToGatewayUrl(data.imageIpfsUri)}
            alt={data.title}
            className="w-full object-contain"
          />
        </div>

        <div className="space-y-4">
          <div>
            <h1 className="font-serif text-2xl font-semibold">{data.title}</h1>
            <p className="text-ink/60">Token #{data.tokenId}</p>
          </div>

          <div className="rounded-md border border-ink/10 bg-white p-4">
            <p className="text-sm text-ink/50">Content hash (SHA-256)</p>
            <p className="break-all font-mono text-xs">{data.contentHashHex}</p>
          </div>

          <div className="flex flex-wrap gap-3">
            <a
              href={certificatePdfUrl(data.tokenId)}
              className="rounded-full bg-brass px-4 py-2 text-sm font-semibold text-white hover:bg-brass/90"
            >
              {t('certificate.downloadPdf', 'Download Certificate (PDF)')}
            </a>
            <a
              href={data.openSeaUrl}
              target="_blank"
              rel="noreferrer"
              className="rounded-full border border-ink/20 px-4 py-2 text-sm font-semibold text-ink hover:bg-ink/5"
            >
              {t('certificate.viewOnOpenSea', 'List on OpenSea')}
            </a>
            <a
              href={data.raribleUrl}
              target="_blank"
              rel="noreferrer"
              className="rounded-full border border-ink/20 px-4 py-2 text-sm font-semibold text-ink hover:bg-ink/5"
            >
              {t('certificate.viewOnRarible', 'View on Rarible')}
            </a>
            <a
              href={data.etherscanUrl}
              target="_blank"
              rel="noreferrer"
              className="rounded-full border border-ink/20 px-4 py-2 text-sm font-semibold text-ink hover:bg-ink/5"
            >
              {t('certificate.viewOnEtherscan', 'View Transaction on Etherscan')}
            </a>
          </div>
        </div>
      </div>

      <div className="rounded-md border border-ink/10 bg-white p-4">
        <h2 className="mb-2 font-serif text-lg font-semibold">Certificate details</h2>
        <div className="flex items-center justify-between gap-4 border-b border-ink/10 py-3">
          <span className="text-sm text-ink/50">Contract</span>
          <span className="truncate text-right font-mono text-xs">{data.contractAddress}</span>
        </div>
        <LinkRow label="Image (IPFS)" href={ipfsToGatewayUrl(data.imageIpfsUri)} mono />
        <LinkRow label="Metadata (IPFS)" href={ipfsToGatewayUrl(data.metadataIpfsUri)} mono />
        <div className="flex items-center justify-between gap-4 py-3">
          <span className="text-sm text-ink/50">Owner</span>
          <span className="font-mono text-xs">{data.ownerAddress}</span>
        </div>
        <div className="flex items-center justify-between gap-4 py-3">
          <span className="text-sm text-ink/50">Royalty</span>
          <span className="text-sm">{(data.royaltyPercentageBps / 100).toFixed(2)}%</span>
        </div>
      </div>

      <div className="rounded-md border border-brass/30 bg-brass/5 p-4 text-sm text-ink/70">
        <h2 className="mb-1 font-serif text-base font-semibold text-ink">
          {t('certificate.authenticity.title', 'How this certificate protects you')}
        </h2>
        <p>
          {t(
            'certificate.authenticity.body',
            "Your photograph's content hash and ownership are permanently recorded on a public blockchain, making forgery and unauthorized resale without royalty payment provable and preventable.",
          )}
        </p>
      </div>
    </div>
  );
}
