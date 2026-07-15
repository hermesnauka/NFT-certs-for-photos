'use client';

import { FormEvent, useState } from 'react';
import { useRouter } from 'next/navigation';
import { useAccount } from 'wagmi';
import { createArtwork, mintCertificate, uploadFile, verifyIdentity } from '../lib/api';
import { ApiError, ArtworkResponse, CertificateDto, UploadResponse } from '../lib/types';
import { useTranslation } from '../lib/i18n/context';
import { Dropzone, SelectedFile } from './Dropzone';
import { ConnectWalletButton } from './ConnectWalletButton';

type Step = 'identity' | 'upload' | 'metadata' | 'review' | 'success';

const STEP_LABELS: Record<Step, string> = {
  identity: 'Verify identity',
  upload: 'Upload photo',
  metadata: 'Describe artwork',
  review: 'Review & mint',
  success: 'Done',
};

function errorMessage(error: unknown, fallback: string): string {
  if (error instanceof ApiError) return error.message;
  if (error instanceof Error) return error.message;
  return fallback;
}

export function MintWizard() {
  const { address, isConnected } = useAccount();
  const { t } = useTranslation();
  const router = useRouter();

  const [step, setStep] = useState<Step>('identity');
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const [did, setDid] = useState('did:key:z6Mk');
  const [email, setEmail] = useState('');

  const [selectedFile, setSelectedFile] = useState<SelectedFile | null>(null);
  const [upload, setUpload] = useState<UploadResponse | null>(null);

  const [title, setTitle] = useState('');
  const [description, setDescription] = useState('');
  const [medium, setMedium] = useState('');
  const [yearCreated, setYearCreated] = useState('');
  const [royaltyPercent, setRoyaltyPercent] = useState('7.5');
  const [artwork, setArtwork] = useState<ArtworkResponse | null>(null);

  const [certificate, setCertificate] = useState<CertificateDto | null>(null);

  if (!isConnected || !address) {
    return (
      <div className="rounded-lg border border-dashed border-ink/20 p-12 text-center">
        <p className="mb-4 text-ink/70">Connect your wallet to start minting a certificate.</p>
        <div className="flex justify-center">
          <ConnectWalletButton />
        </div>
      </div>
    );
  }

  async function handleVerifyIdentity(event: FormEvent) {
    event.preventDefault();
    setBusy(true);
    setError(null);
    try {
      const result = await verifyIdentity(address as string, did, email);
      if (!result.verified) {
        setError('Identity verification did not succeed.');
        return;
      }
      setStep('upload');
    } catch (err) {
      setError(errorMessage(err, 'Could not verify identity.'));
    } finally {
      setBusy(false);
    }
  }

  async function handleUploadContinue() {
    if (!selectedFile) return;
    setBusy(true);
    setError(null);
    try {
      const result = await uploadFile(selectedFile.file);
      setUpload(result);
      setStep('metadata');
    } catch (err) {
      setError(errorMessage(err, 'Could not upload file.'));
    } finally {
      setBusy(false);
    }
  }

  async function handleMetadataSubmit(event: FormEvent) {
    event.preventDefault();
    if (!upload) return;
    setBusy(true);
    setError(null);
    try {
      const royaltyBps = Math.round(parseFloat(royaltyPercent || '0') * 100);
      const result = await createArtwork({
        fileId: upload.fileId,
        title,
        description: description || undefined,
        medium: medium || undefined,
        yearCreated: yearCreated ? parseInt(yearCreated, 10) : undefined,
        royaltyPercentageBps: royaltyBps,
        artistWalletAddress: address as string,
        artistDid: did,
      });
      setArtwork(result);
      setStep('review');
    } catch (err) {
      setError(errorMessage(err, 'Could not save artwork metadata.'));
    } finally {
      setBusy(false);
    }
  }

  async function handleMint() {
    if (!artwork) return;
    setBusy(true);
    setError(null);
    try {
      const result = await mintCertificate(artwork.artworkId, address as string);
      setCertificate(result);
      setStep('success');
    } catch (err) {
      setError(errorMessage(err, 'Minting failed.'));
    } finally {
      setBusy(false);
    }
  }

  return (
    <div className="space-y-6">
      <ol className="flex flex-wrap gap-2 text-xs text-ink/50">
        {(['identity', 'upload', 'metadata', 'review', 'success'] as Step[]).map((s, i) => (
          <li
            key={s}
            className={`rounded-full px-3 py-1 ${step === s ? 'bg-ink/10 font-semibold text-ink' : ''}`}
          >
            {i + 1}. {STEP_LABELS[s]}
          </li>
        ))}
      </ol>

      {error && (
        <div className="rounded-md border border-red-200 bg-red-50 px-4 py-3 text-sm text-red-700">
          {error}
        </div>
      )}

      {step === 'identity' && (
        <form onSubmit={handleVerifyIdentity} className="max-w-md space-y-4">
          <h2 className="font-serif text-xl font-semibold">
            {t('identity.verify.title', 'Verify Your Identity')}
          </h2>
          <p className="text-sm text-ink/60">
            {t(
              'identity.verify.explainer',
              'We confirm you are the rightful copyright holder of this photograph before any certificate can be minted in your name.',
            )}
          </p>
          <label className="block text-sm">
            Decentralized Identifier (DID)
            <input
              required
              value={did}
              onChange={(e) => setDid(e.target.value)}
              className="mt-1 w-full rounded-md border border-ink/20 px-3 py-2"
              placeholder="did:key:z6Mk..."
            />
          </label>
          <label className="block text-sm">
            Email
            <input
              required
              type="email"
              value={email}
              onChange={(e) => setEmail(e.target.value)}
              className="mt-1 w-full rounded-md border border-ink/20 px-3 py-2"
            />
          </label>
          <button
            type="submit"
            disabled={busy}
            className="rounded-full bg-brass px-5 py-2 text-sm font-semibold text-white hover:bg-brass/90 disabled:opacity-50"
          >
            {busy ? 'Working…' : 'Verify & continue'}
          </button>
        </form>
      )}

      {step === 'upload' && (
        <div className="max-w-md space-y-4">
          <h2 className="font-serif text-xl font-semibold">Upload your photograph</h2>
          <Dropzone onSelect={setSelectedFile} />
          {selectedFile && (
            <div className="space-y-2 rounded-md border border-ink/10 bg-white p-4 text-sm">
              {/* eslint-disable-next-line @next/next/no-img-element */}
              <img
                src={selectedFile.previewUrl}
                alt={selectedFile.file.name}
                className="max-h-48 rounded-md object-contain"
              />
              <p className="truncate text-ink/70">{selectedFile.file.name}</p>
              <p className="break-all font-mono text-xs text-ink/50">{selectedFile.clientHash}</p>
              <p className="text-xs text-ink/50">
                {t(
                  'mint.hash.explainer',
                  "This SHA-256 hash is your artwork's unique digital fingerprint. It cryptographically proves the file has not been altered and links it to your certificate forever.",
                )}
              </p>
            </div>
          )}
          <button
            type="button"
            disabled={!selectedFile || busy}
            onClick={handleUploadContinue}
            className="rounded-full bg-brass px-5 py-2 text-sm font-semibold text-white hover:bg-brass/90 disabled:opacity-50"
          >
            {busy ? 'Working…' : 'Continue'}
          </button>
        </div>
      )}

      {step === 'metadata' && upload && (
        <form onSubmit={handleMetadataSubmit} className="max-w-md space-y-4">
          <h2 className="font-serif text-xl font-semibold">Describe your artwork</h2>
          <label className="block text-sm">
            {t('mint.title.label', 'Title')}
            <input
              required
              value={title}
              onChange={(e) => setTitle(e.target.value)}
              className="mt-1 w-full rounded-md border border-ink/20 px-3 py-2"
            />
          </label>
          <label className="block text-sm">
            {t('mint.description.label', 'Description')}
            <textarea
              value={description}
              onChange={(e) => setDescription(e.target.value)}
              className="mt-1 w-full rounded-md border border-ink/20 px-3 py-2"
              rows={3}
            />
          </label>
          <div className="grid grid-cols-2 gap-4">
            <label className="block text-sm">
              {t('mint.medium.label', 'Medium')}
              <input
                value={medium}
                onChange={(e) => setMedium(e.target.value)}
                className="mt-1 w-full rounded-md border border-ink/20 px-3 py-2"
              />
            </label>
            <label className="block text-sm">
              {t('mint.year.label', 'Year of Creation')}
              <input
                type="number"
                value={yearCreated}
                onChange={(e) => setYearCreated(e.target.value)}
                className="mt-1 w-full rounded-md border border-ink/20 px-3 py-2"
              />
            </label>
          </div>
          <label className="block text-sm">
            {t('mint.royalty.label', 'Royalty Percentage')}
            <input
              required
              type="number"
              min={0}
              max={100}
              step={0.01}
              value={royaltyPercent}
              onChange={(e) => setRoyaltyPercent(e.target.value)}
              className="mt-1 w-full rounded-md border border-ink/20 px-3 py-2"
            />
          </label>
          <button
            type="submit"
            disabled={busy}
            className="rounded-full bg-brass px-5 py-2 text-sm font-semibold text-white hover:bg-brass/90 disabled:opacity-50"
          >
            {busy ? 'Working…' : 'Continue'}
          </button>
        </form>
      )}

      {step === 'review' && artwork && (
        <div className="max-w-md space-y-4">
          <h2 className="font-serif text-xl font-semibold">Review & mint</h2>
          <dl className="space-y-2 rounded-md border border-ink/10 bg-white p-4 text-sm">
            <div className="flex justify-between gap-4">
              <dt className="text-ink/50">Title</dt>
              <dd className="truncate text-right">{title}</dd>
            </div>
            <div className="flex justify-between gap-4">
              <dt className="text-ink/50">Content hash</dt>
              <dd className="truncate text-right font-mono text-xs">{artwork.sha256Hash}</dd>
            </div>
            <div className="flex justify-between gap-4">
              <dt className="text-ink/50">Status</dt>
              <dd className="text-right">{artwork.status}</dd>
            </div>
            <div className="flex justify-between gap-4">
              <dt className="text-ink/50">Recipient wallet</dt>
              <dd className="truncate text-right font-mono text-xs">{address}</dd>
            </div>
          </dl>
          <button
            type="button"
            disabled={busy}
            onClick={handleMint}
            className="rounded-full bg-brass px-5 py-2 text-sm font-semibold text-white hover:bg-brass/90 disabled:opacity-50"
          >
            {busy ? 'Working…' : t('mint.submit', 'Mint Certificate')}
          </button>
        </div>
      )}

      {step === 'success' && certificate && (
        <div className="max-w-md space-y-4 rounded-md border border-ink/10 bg-white p-6 text-center">
          <h2 className="font-serif text-xl font-semibold">Certificate minted!</h2>
          <p className="text-ink/60">Token ID #{certificate.tokenId}</p>
          <button
            type="button"
            onClick={() => router.push(`/certificate/${certificate.tokenId}`)}
            className="rounded-full bg-brass px-5 py-2 text-sm font-semibold text-white hover:bg-brass/90"
          >
            View certificate
          </button>
        </div>
      )}
    </div>
  );
}
