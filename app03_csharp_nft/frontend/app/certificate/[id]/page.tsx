import { notFound } from 'next/navigation';
import { getCertificate } from '../../../lib/api';
import { ApiError } from '../../../lib/types';
import { CertificateView } from '../../../components/CertificateView';

export default async function CertificatePage({ params }: { params: { id: string } }) {
  try {
    const certificate = await getCertificate(params.id);
    return <CertificateView data={certificate} />;
  } catch (err) {
    if (err instanceof ApiError && err.status === 404) {
      notFound();
    }
    throw err;
  }
}
