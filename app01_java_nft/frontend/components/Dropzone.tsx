'use client';

import { useCallback, useRef, useState } from 'react';
import { useTranslation } from '../lib/i18n/context';

const ACCEPTED_TYPES = ['image/jpeg', 'image/png', 'image/webp'];

async function sha256Hex(file: File): Promise<string> {
  const buffer = await file.arrayBuffer();
  const digest = await crypto.subtle.digest('SHA-256', buffer);
  return Array.from(new Uint8Array(digest))
    .map((byte) => byte.toString(16).padStart(2, '0'))
    .join('');
}

export interface SelectedFile {
  file: File;
  previewUrl: string;
  clientHash: string;
}

export function Dropzone({ onSelect }: { onSelect: (selected: SelectedFile) => void }) {
  const { t } = useTranslation();
  const [isDragging, setIsDragging] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [hashing, setHashing] = useState(false);
  const inputRef = useRef<HTMLInputElement>(null);

  const handleFile = useCallback(
    async (file: File) => {
      setError(null);
      if (!ACCEPTED_TYPES.includes(file.type)) {
        setError(t('mint.upload.unsupportedType', 'Unsupported file type. Use JPEG, PNG, or WebP.'));
        return;
      }
      setHashing(true);
      try {
        const clientHash = await sha256Hex(file);
        onSelect({ file, previewUrl: URL.createObjectURL(file), clientHash });
      } finally {
        setHashing(false);
      }
    },
    [onSelect, t],
  );

  return (
    <div
      onDragOver={(event) => {
        event.preventDefault();
        setIsDragging(true);
      }}
      onDragLeave={() => setIsDragging(false)}
      onDrop={(event) => {
        event.preventDefault();
        setIsDragging(false);
        const file = event.dataTransfer.files?.[0];
        if (file) void handleFile(file);
      }}
      onClick={() => inputRef.current?.click()}
      role="button"
      tabIndex={0}
      className={`flex cursor-pointer flex-col items-center justify-center rounded-lg border-2 border-dashed p-12 text-center transition ${
        isDragging ? 'border-brass bg-brass/5' : 'border-ink/20 hover:border-ink/40'
      }`}
    >
      <input
        ref={inputRef}
        type="file"
        accept={ACCEPTED_TYPES.join(',')}
        className="hidden"
        onChange={(event) => {
          const file = event.target.files?.[0];
          if (file) void handleFile(file);
        }}
      />
      <p className="text-ink/70">
        {hashing
          ? t('mint.upload.hashing', 'Computing digital fingerprint…')
          : t('mint.dropzone.label', 'Drag and drop your photograph here, or click to browse')}
      </p>
      {error && <p className="mt-2 text-sm text-red-700">{error}</p>}
    </div>
  );
}
